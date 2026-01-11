#include "usecases/messageusecase.h"

#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

#include "managers/TokenManager.h"
#include "managers/datamanager.h"
#include "managers/messagemanager.h"
#include "models/messagemodel.h"

namespace {

template <typename T>
T waitForFuture(QFuture<T> &future) {
  QFutureWatcher<T> watcher;
  watcher.setFuture(future);

  QEventLoop loop;
  QObject::connect(&watcher, &QFutureWatcher<T>::finished, &loop, &QEventLoop::quit);
  loop.exec();

  return future.result();
}

}  // namespace

MessageUseCase::MessageUseCase(DataManager *data_manager, std::unique_ptr<MessageManager> message_manager,
                               TokenManager *token_manager)
    : data_manager_(data_manager), message_manager_(std::move(message_manager)), token_manager_(token_manager) {
  connect(data_manager_, &DataManager::messageAdded, this, [&](const Message &added_messaage) {
    LOG_INFO("Received DataManager::messageAdded (text is {})", added_messaage.text.toStdString());
    DBC_REQUIRE(added_messaage.chat_id > 0);
    auto message_model = data_manager_->getMessageModel(added_messaage.chat_id);
    DBC_REQUIRE(message_model);
    message_model->saveMessage(added_messaage);
    if (added_messaage.id != 0) Q_EMIT messageAdded(added_messaage);  // this message from server, not offline
  });
}

auto MessageUseCase::getChatMessages(long long chat_id, int limit) -> QList<Message> {
  auto message_model = data_manager_->getMessageModel(chat_id);
  DBC_REQUIRE(message_model != nullptr);
  auto oldestMessage = message_model->getOldestMessage();

  long long id_of_oldest_message = oldestMessage.has_value() ? oldestMessage->id : 0ll;

  // TODO: cache request result for {chat_id before_id}
  LOG_INFO(
      "[getChatMessages] Loading messages for chatId={}, "
      "id_of_oldest_message = '{}'",
      chat_id, id_of_oldest_message);
  auto future = message_manager_->getChatMessages(token_manager_->getToken(), chat_id, id_of_oldest_message, limit);
  return waitForFuture(future);
}

MessageModel *MessageUseCase::getMessageModel(long long chat_id) {
  auto message_model = data_manager_->getMessageModel(chat_id);
  assert(message_model);
  return message_model.get();
}

void MessageUseCase::addMessageToChat(Message &msg) {
  // 1) Add message in data_manager
  // 2) Connect DataManagerMessageAdded signal
  // 3) When added -> message_model_->addMessage() + signal message Added
  // 4) Model Connect MessageUseCase::MessageAdded
  // 5) Model try to load user of message, and try to load chat history if chat
  // not finded

  PROFILE_SCOPE("MessageUseCase::addMessageToChat");
  DBC_REQUIRE(!msg.local_id.isEmpty());
  DBC_REQUIRE(msg.sender_id > 0);
  long long current_id = token_manager_->getCurrentUserId();
  if (current_id == msg.sender_id) msg.is_mine = true;
  data_manager_->saveMessage(msg);
}

void MessageUseCase::updateMessage(Message &msg) {
  long long current_id = token_manager_->getCurrentUserId();
  if (current_id == msg.sender_id) msg.is_mine = true;
  data_manager_->saveMessage(msg);
  message_manager_->updateMessage(msg, token_manager_->getToken());
}

void MessageUseCase::deleteMessage(const Message &msg) {
  data_manager_->deleteMessage(msg);
  message_manager_->deleteMessage(msg, token_manager_->getToken());
}

void MessageUseCase::logout() {
  PROFILE_SCOPE("MessageUseCase::logout");
  clearAllMessages();
  LOG_INFO("[logout] Logout message use case is completed");
}

void MessageUseCase::clearAllMessages() {
  data_manager_->clearAllMessageModels();
  DBC_ENSURE(data_manager_->getNumberOfMessageModels() == 0);
}

void MessageUseCase::getChatMessagesAsync(long long chat_id) {
  PROFILE_SCOPE("MessageUseCase::getChatMessagesAsync");
  DBC_REQUIRE(chat_id > 0);

  // auto watcher = new QFutureWatcher<QList<Message>>(this);

  // connect(
  //     watcher, &QFutureWatcher<QList<Message>>::finished, this,
  //     [this, watcher, chat_id]() {
  //       try {
  //         auto chat_messages = watcher->result();
  //         LOG_INFO("[getChatMessagesAsync] For chat '{}' loaded '{}' messages",
  //                  chat_id, chat_messages.size());
  //         for (auto &message : chat_messages) { // todo(roma): make pipeline
  //           addMessageToChat(message);
  //         }

  //       } catch (...) {
  //         LOG_ERROR("Error in getChatMessagesAsync for chat_id {}", chat_id);
  //       }

  //       watcher->deleteLater();
  //     });

  // QFuture<QList<Message>> future = QtConcurrent::run([this, chat_id]() {
  //   return getChatMessages(
  //       chat_id); // todo: make manager_->getChatMessagesAsync(?)
  // });

  // watcher->setFuture(future);

  QtConcurrent::run([this, chat_id]() { return getChatMessages(chat_id); })
      .then(this,
            [this, chat_id](QList<Message> chat_messages) {
              LOG_INFO("[getChatMessagesAsync] For chat '{}' loaded '{}' messages", chat_id, chat_messages.size());

              for (auto &message : chat_messages) {
                addMessageToChat(message);
              }
            })
      .onFailed(this, [chat_id]() { LOG_ERROR("Error in getChatMessagesAsync for chat_id {}", chat_id); });
}
