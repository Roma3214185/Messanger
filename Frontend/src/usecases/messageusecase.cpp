#include "usecases/messageusecase.h"

#include <QFuture>
#include <QFutureWatcher>

#include "managers/datamanager.h"
#include "models/messagemodel.h"
#include "managers/messagemanager.h"
#include "managers/TokenManager.h"

namespace {

template <typename T>
T waitForFuture(QFuture<T>& future) {
  QFutureWatcher<T> watcher;
  watcher.setFuture(future);

  QEventLoop loop;
  QObject::connect(&watcher, &QFutureWatcher<T>::finished, &loop, &QEventLoop::quit);
  loop.exec();

  return future.result();
}

}  // namespace

MessageUseCase::MessageUseCase(DataManager* data_manager, MessageManager* message_manager, TokenManager* token_manager)
    : data_manager_(data_manager), message_manager_(message_manager), token_manager_(token_manager) {}

auto MessageUseCase::getChatMessages(long long chat_id, int limit) -> QList<Message> {
  int before_id = 0;
  //TODO: cache request result for {chat_id before_id}

  auto message_model = data_manager_->getMessageModel(chat_id);
  if (message_model) {
    auto oldestMessage = message_model->getOldestMessage();
    if (oldestMessage) {
      LOG_INFO("Last message with id '{}' and text '{}'",
               oldestMessage->id,
               oldestMessage->text.toStdString());
      before_id = oldestMessage->id;
    }
  }

  LOG_INFO("[getChatMessages] Loading messages for chatId={}, beforeId = '{}'", chat_id, before_id);
  auto future = message_manager_->getChatMessages(token_manager_->getToken(), chat_id, before_id, limit);
  return waitForFuture(future);
}

MessageModel* MessageUseCase::getMessageModel(long long chat_id) {
  PROFILE_SCOPE("MessageUseCase::getMessageModel");
  auto message_model = data_manager_->getMessageModel(chat_id);
  if (!message_model) {
    LOG_ERROR("Message model is nullptr for id {}", chat_id);
    throw std::runtime_error("Nullptr messagemodel");
  }
  LOG_INFO("Message model is getted from Model");

  return message_model.get();
}

// void MessageUseCase::addMessageToChat(long long chat_id, const Message& msg) {
//   PROFILE_SCOPE("MessageUseCase::addMessageToChat");
//   auto chat = data_manager_->getChat(chat_id);
//   if (!chat) {
//     LOG_INFO("Chat with id '{}' isn't exist", chat_id);
//     auto chat = loadChat(msg.chatId);  // u can receive new message from group/user if u
//         // delete for youtself and from newUser
//     addChatInFront(chat);
//   }

//   auto message_model = data_manager_->getMessageModel(chat_id);
//   if(!message_model) {
//     qDebug() << "Nullptr message model"; //todo(roma): implement solutions for this case
//     return;
//   }
//   auto user = data_manager_->getUser(msg.senderId);

//   if (!user) {
//     LOG_INFO("There is no info about user {} in cache", msg.senderId);
//     auto user_from_server = getUser(msg.senderId);
//     if(!user_from_server) {
//       LOG_ERROR("Server can't find info about user {}", msg.senderId);
//       return;
//     }
//     data_manager_->saveUser(*user_from_server);
//     user = user_from_server;
//   } else {
//     getUserAsync(msg.senderId);
//   }

//   addMessageWithUpdatingChatList(msg, *user, chat_id, message_model);
// }

void MessageUseCase::addMessageWithUpdatingChatList(const Message& msg, const User& user, long long chat_id, MessageModelPtr message_model) {
  if(!message_model) throw std::runtime_error("Message_model is nulltpr");
  message_model->addMessage(msg, user);
  auto last_chat_message = message_model->getLastMessage();
  Q_EMIT messageAdded(msg);

  //  todo(roma): model get's this -> get's from here last message of chat, and
  //  chat_model_->updateChatInfo(chat_id, last_chat_message);
}

void MessageUseCase::addOfflineMessageToChat(long long chat_id, const User& user, const Message& msg) {
  auto message_model = data_manager_->getMessageModel(chat_id);
  if(!message_model) {   // TODO: make one function add message(offline + online)
    LOG_ERROR("Invalid message_model");
    return;
  }

  addMessageWithUpdatingChatList(msg, user, chat_id, message_model);
}


void MessageUseCase::logout() {
  PROFILE_SCOPE("MessageUseCase::logout");
  clearAllMessages();
  LOG_INFO("[logout] Logout message use case is completed");
}

void MessageUseCase::clearAllMessages() {
  data_manager_->clearAllMessageModels();
}

// void MessageUseCase::onMessageReceived(const QString& msg) {
//   PROFILE_SCOPE("MessageUseCase::onMessageReceived");
//   LOG_INFO("[onMessageReceived] Message received from user {}: ", msg.toStdString());

//   QJsonParseError parseError;
//   auto            doc = QJsonDocument::fromJson(msg.toUtf8(), &parseError);

//   if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
//     LOG_ERROR("[onMessageReceived] Failed JSON parse: '{}'",
//               parseError.errorString().toStdString());
//     Q_EMIT errorOccurred("Invalid JSON received: " + parseError.errorString());
//     return;
//   }

//   auto json_responce = doc.object();
//   Q_EMIT newResponce(json_responce);
// }

void MessageUseCase::fillChatHistory(long long chat_id) {
  PROFILE_SCOPE("MessageUseCase::fillChatHistory");
  auto message_history = getChatMessages(chat_id); //implement here signal to each loaded message -> signal to add in message_model
  LOG_INFO("[fillChatHistory] For chat '{}' loaded '{}' messages", chat_id, message_history.size());
  auto message_model = data_manager_->getMessageModel(chat_id);

  // if (message_history.empty()) {
  //   return;
  // }

  // for (auto message : message_history) {
  //   auto user = getUser(message.senderId);
  //   if (!user) {
  //     LOG_ERROR("[fillChatHistory] getUser failed for message '{}'", message.id);
  //   } else {
  //     LOG_INFO("[fillChatHistory] For message '{}' user is '{}'", message.id, user->id);
  //   }

  //   message_model->addMessage(message, *user);
  // }

  // auto last_message = message_model->getLastMessage();
  // chat_model_->updateChatInfo(chat_id, last_message);
}

