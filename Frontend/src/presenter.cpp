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
  QString       type           = json_object["type"].toString();
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

  auto newMessages = manager_->getChatMessages(chat_id, 20);
  if (newMessages.empty()) return;

  auto message_model = manager_->getMessageModel(chat_id);
  assert(message_model);

  message_list_view_->preserveFocusWhile(message_model, [&]{
    for (const auto& msg : newMessages) {
      manager_->addMessageToChat(chat_id, msg); //TODO: make pipeline
    }
  });

  /*  TODO: auto message_model = manager_->getMessageModel(chat_id);
  auto future = manager_->getChatMessages(chat_id, 20);
  future.then([this](auto newMessages) {
    message_list_view_->preserveFocusWhile(message_model, [&]{
      for (const auto& msg : newMessages)
        manager_->addMessageToChat(chat_id, msg); //TODO: make pipeline
    });
  });*/
}

void Presenter::onErrorOccurred(const QString& error) { view_->showError(error); }

void Presenter::setUser(const User& user, const QString& token) {
  PROFILE_SCOPE("Presenter::setUser");
  LOG_INFO("Set user name: '{}' | email '{}' | tag '{}' id '{}'",
           user.name.toStdString(),
           user.email.toStdString(),
           user.tag.toStdString(),
           user.id);

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
  current_opened_chat_id_ = chat_id;
}

void Presenter::onChatClicked(int chat_id) { openChat(chat_id); }

void Presenter::newMessage(Message& msg) {
  if(!current_user_) {
    LOG_ERROR("User isn't initialized");
    return;
  }

  if (msg.senderId == current_user_->id) msg.readed_by_me = true;
  LOG_INFO("New messages for chat {} :({}) with local_id {}",
           msg.chatId,
           msg.text.toStdString(),
           msg.local_id.toStdString());

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
  LOG_INFO("Message model is getted");
  message_list_view_->setMessageModel(message_model);
  LOG_INFO("Message model is setted");
  message_list_view_->scrollToBottom();
  LOG_INFO("Scroll to bottom");
  auto chat = manager_->getChat(chat_id);
  LOG_INFO("Getted chat");
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
  if(!current_opened_chat_id_) {
    LOG_ERROR("There is no opened chat");
    return;
  }

  if(!current_user_) {
    LOG_ERROR("User is no initailized");
    return;
  }

  if (text_to_send.isEmpty()) {
    LOG_WARN("Presenter receive to send empty text");
    return;
  }

  //TODO: what if multithreaded will make here current_user is nullopt, after checking (?)

  Message message_to_send{.chatId        = *current_opened_chat_id_,
                          .senderId      = current_user_->id,
                          .text          = text_to_send,
                          .status_sended = false,
                          .timestamp     = QDateTime::currentDateTime(),
                          .local_id      = QUuid::createUuid().toString()};
  LOG_INFO("Set for message {} local_id {}",
           message_to_send.text.toStdString(),
           message_to_send.local_id.toStdString());
  manager_->addOfflineMessageToChat(message_to_send.chatId, *current_user_, message_to_send);
  message_list_view_->scrollToBottom();
  manager_->sendMessage(message_to_send);
}

void Presenter::onLogOutButtonClicked() {
  manager_->logout();
  current_user_ = std::nullopt;
  current_opened_chat_id_ = std::nullopt;
}
