#include "presenter.h"

#include <QJsonObject>
#include <QtConcurrent/QtConcurrent>

#include "Debug_profiling.h"
#include "JsonService.h"
#include "MessageListView.h"
#include "dto/SignUpRequest.h"
#include "dto/User.h"
#include "interfaces/IMainWindow.h"
#include "model.h"
#include "models/messagemodel.h"
#include "interfaces/IMessageListView.h"
#include <iostream>

namespace {

void debug(const QString& log, const User& user) {
  LOG_INFO("{}: User '{}' | email '{}' | tag '{}' id '{}'",
           log.toStdString(),
           user.name.toStdString(),
           user.email.toStdString(),
           user.tag.toStdString(),
           user.id);
}

void debug(const QString& log, const Message& message) {
  LOG_INFO("{}: Message chat_id '{}' | sender_id '{}' | text '{}' timestamp '{}', local_id {}",
           log.toStdString(),
           message.chatId,
           message.senderId,
           message.text.toStdString(),
           message.timestamp.toString().toStdString(),
           message.local_id.toStdString());
}

bool checkOpenedChatAndUser(const std::optional<int>& current_opened_chat_id_, const std::optional<User>& current_user_) {
  if(!current_opened_chat_id_) {
    LOG_ERROR("There is no opened chat");
    return false;
  }

  if(!current_user_) {
    LOG_ERROR("User is no initailized");
    return false;
  }

  return true;
}

class EntityFactory {
  public:
    static Message createMessage(int chat_id, int sender_id, const QString& text, const QString& local_id, QDateTime timestamp = QDateTime::currentDateTime()) {
      Message message{.chatId        = chat_id,
                      .senderId      = sender_id,
                      .text          = text,
                      .status_sended = false,
                      .timestamp     = timestamp,
                      .local_id      = local_id};
      return message;
    }
};

} // namespace

Presenter::Presenter(IMainWindow* window, Model* manager) : view_(window), manager_(manager) {}

void Presenter::initialise() {
  view_->setChatModel(manager_->getChatModel());
  view_->setUserModel(manager_->getUserModel());
  manager_->checkToken();

  initialConnections();
}

void Presenter::setMessageListView(IMessageListView* message_list_view) {
  message_list_view_ = message_list_view;
  view_->setMessageListView(static_cast<QListView*>(message_list_view));
}

void Presenter::signIn(const LogInRequest& login_request) { manager_->signIn(login_request); }

void Presenter::signUp(const SignUpRequest& req) { manager_->signUp(req); }

void Presenter::initialConnections() {
  connect(manager_, &Model::userCreated, this, &Presenter::setUser);
  connect(manager_, &Model::newResponce, this, &Presenter::onNewResponce);
  connect(
      manager_, &Model::chatAdded, this, [this](int chatId) { manager_->fillChatHistory(chatId); });
  connect(manager_, &Model::errorOccurred, this, &Presenter::onErrorOccurred);

  if (!message_list_view_) {
    LOG_ERROR("MessageListView is nullptr in initial connections");
    throw std::runtime_error("Nullptr in Presenter::connections");
  }

  connect(message_list_view_, &IMessageListView::scrollChanged, this, &Presenter::onScroll);
  connect(manager_, &Model::chatUpdated, this, &Presenter::onChatUpdated);
}

void Presenter::onNewResponce(QJsonObject& json_object) {
  const QString type           = json_object["type"].toString();
  const QString openedType     = "opened";
  const QString newMessageType = "new_message";

  if(!current_user_) throw std::runtime_error("User isn't inialised");
  if (type == openedType) {
    LOG_INFO("Open type");
    manager_->initSocket(current_user_->id);
  } else if (type == newMessageType) {
    LOG_INFO("New Message type");
    auto new_message = JsonService::getMessageFromJson(json_object);
    newMessage(new_message);
  } else {
    LOG_ERROR("Invalid type");
  }
}

void Presenter::onChatUpdated(int chatId) {
  // if (!current_chat_id_) return;
  // QModelIndex idx = manager_->indexByChatId(chatId);
  // if (idx.isValid()) {
  //   view_->setCurrentChatIndex(idx);
  //   // ui->chatListView->scrollTo(idx, QAbstractItemView::PositionAtCenter);
  // }
}

void Presenter::onScroll(int value) {
  bool chat_list_is_on_top = (value == 0);
  if (!chat_list_is_on_top) return;
  if(!current_opened_chat_id_) {
    LOG_ERROR("Chat not opened");
    return;
  }

  PROFILE_SCOPE("Presenter::onScroll");
  int chat_id = *current_opened_chat_id_;
  constexpr int limit_of_loading_messages = 20;
  auto newMessages = manager_->getChatMessages(chat_id, limit_of_loading_messages);
  if (newMessages.empty()) return;

  auto message_model = manager_->getMessageModel(chat_id);
  assert(message_model);

  message_list_view_->preserveFocusWhile(message_model, [&]{
    for (const auto& msg : newMessages) {
      manager_->addMessageToChat(chat_id, msg); //TODO: make pipeline
    }
  });
  // TODO: think about future / then
}

void Presenter::onErrorOccurred(const QString& error) { view_->showError(error); }

void Presenter::setUser(const User& user, const QString& token) {
  PROFILE_SCOPE("Presenter::setUser");
  debug("In set user:", user);

  current_user_ = user;
  view_->setUser(user);
  manager_->setCurrentUserId(user.id);
  manager_->saveToken(token);

  auto chats = manager_->loadChats();
  LOG_INFO("In presenter loaded '{}' chats for user id '{}'", chats.size(), user.id);

  for (const auto& chat : chats) {
    manager_->addChat(chat);
  }

  manager_->connectSocket();
}

void Presenter::setCurrentChatId(int chat_id) {
  assert(chat_id > 0);
  current_opened_chat_id_ = chat_id;
}

void Presenter::onChatClicked(int chat_id) { openChat(chat_id); }

void Presenter::newMessage(Message& msg) {
  if(!current_user_) {
    LOG_ERROR("User isn't initialized");
    return;
  }

  if (msg.senderId == current_user_->id) msg.readed_by_me = true;
  debug("New message received from socket", msg);

  int max   = message_list_view_->getMaximumMessageScrollBar();
  int value = message_list_view_->getMessageScrollBarValue();

  manager_->addMessageToChat(msg.chatId, msg);

  if (current_opened_chat_id_.has_value() && current_opened_chat_id_ == msg.chatId && max == value) {
    message_list_view_->scrollToBottom();
  }
}

void Presenter::findUserRequest(const QString& text) {
  if (text.isEmpty()) {
    manager_->getUserModel()->clear();
    return;
  }

  if(!current_user_) {
    LOG_ERROR("User isn't inislized");
    return;
  }

  auto users = manager_->findUsers(text);
  manager_->getUserModel()->clear();

  for (const auto& user : users) {
    if (current_user_->id != user.id) manager_->getUserModel()->addUser(user);
  }
}

void Presenter::openChat(int chat_id) {  // make unread message = 0; (?)
  PROFILE_SCOPE("Presenter::openChat");
  setCurrentChatId(chat_id);
  auto message_model = manager_->getMessageModel(chat_id);
  message_list_view_->setMessageModel(message_model);
  message_list_view_->scrollToBottom();
  auto chat = manager_->getChat(chat_id);
  view_->setChatWindow(chat);
}

void Presenter::onUserClicked(int user_id, bool is_user) {
  if(!current_user_) {
    LOG_ERROR("User isn't inislized");
    return;
  }

  manager_->getUserModel()->clear();
  view_->clearFindUserEdit();

  if (is_user && current_user_->id == user_id) {
    onErrorOccurred("[ERROR] Impossible to open chat with yourself");
    return;
  }

  if (is_user) {
    auto chat = manager_->getPrivateChatWithUser(user_id);
    if (!chat) {
      onErrorOccurred("Char is null in on_user_clicked");
    } else {
      openChat(chat->chat_id);
    }
  } else {
    qDebug() << "[ERROR] Implement finding group request";
  }
}

void Presenter::sendButtonClicked(const QString& text_to_send) {
  if(!checkOpenedChatAndUser(current_opened_chat_id_, current_user_)) return;

  if (text_to_send.isEmpty()) {
    LOG_WARN("Presenter receive to send empty text");
    return;
  }

  //TODO: what if multithreaded will make here current_user is nullopt, after checking (?)
  auto message_to_send = EntityFactory::createMessage(*current_opened_chat_id_,
                                                      current_user_->id, text_to_send,
                                                      QUuid::createUuid().toString());
  debug("Message to send", message_to_send);
  manager_->addOfflineMessageToChat(message_to_send.chatId, *current_user_, message_to_send);
  message_list_view_->scrollToBottom();
  manager_->sendMessage(message_to_send);
}

void Presenter::onLogOutButtonClicked() {
  manager_->logout();
  current_user_ .reset();
  current_opened_chat_id_.reset();
}
