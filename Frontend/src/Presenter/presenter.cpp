#include "presenter.h"

#include <QtConcurrent/QtConcurrent>

#include "Debug_profiling.h"
#include "MessageModel/messagemodel.h"
#include "Model/model.h"
#include "headers/IMainWindow.h"
#include "headers/MessageListView.h"
#include "headers/SignUpRequest.h"
#include "headers/User.h"

Presenter::Presenter(IMainWindow* window, Model* manager)
    : view_(window), manager_(manager) {
  view_->setChatModel(manager->getChatModel());
  view_->setUserModel(manager->getUserModel());
  manager_->checkToken();

  message_list_view_ = std::make_unique<MessageListView>();
  view_->setMessageListView(message_list_view_.get());

  initialConnections();
}

void Presenter::signIn(const QString& email, const QString& password) {
  LogInRequest login_request{.email = email, .password = password};
  manager_->signIn(login_request);
}

void Presenter::signUp(const SignUpRequest& req) { manager_->signUp(req); }

void Presenter::initialConnections() {
  connect(manager_, &Model::userCreated, this, &Presenter::setUser);
  connect(manager_, &Model::newMessage, this, &Presenter::newMessage);
  connect(manager_, &Model::chatAdded, this,
          [this](int chatId) { manager_->fillChatHistory(chatId); });
  connect(manager_, &Model::errorOccurred, this, &Presenter::onErrorOccurred);

  if (!message_list_view_.get()) {
    LOG_ERROR("MessageListView is nullptr in initial connections");
    throw std::runtime_error("Nullptr in Presenter::connections");
  }

  connect(message_list_view_.get(), &MessageListView::scrollChanged, this,
          &Presenter::onScroll);
  connect(manager_, &Model::chatUpdated, this, &Presenter::onChatUpdated);
}

void Presenter::onChatUpdated(int chatId) {
  if (!current_chat_id_) return;
  QModelIndex idx = manager_->indexByChatId(chatId);
  if (idx.isValid()) {
    view_->setCurrentChatIndex(idx);
    // ui->chatListView->scrollTo(idx, QAbstractItemView::PositionAtCenter);
  }
}

void Presenter::onScroll(int value) {
  if (value != 0) return;

  PROFILE_SCOPE("Presenter::onScroll");
  int chatId = *current_chat_id_;

  auto newMessages = manager_->getChatMessages(chatId, 20);
  if (newMessages.empty()) return;

  for (const auto& newMsg : newMessages) {
    manager_->addMessageToChat(chatId, newMsg, true);
  }

  message_list_view_->verticalScrollBar()->setValue(
      message_list_view_->verticalScrollBar()->value() +
      message_list_view_->sizeHintForRow(0) * newMessages.size());
}

void Presenter::onErrorOccurred(const QString& error) {
  view_->showError(error);
}

void Presenter::setUser(const User& user, const QString& token) {
  PROFILE_SCOPE("Presenter::setUser");
  LOG_INFO("Set user name: '{}' | email '{}' | tag '{}' id '{}'",
           user.name.toStdString(), user.email.toStdString(),
           user.tag.toStdString(), user.id);


  view_->setUser(user);
  setId(user.id);
  manager_->saveToken(token);

  auto chats = manager_->loadChats();
  LOG_INFO("In presenter loaded '{}' chats for user id '{}'", chats.size(),
           user.id);

  for (const auto& chat : chats) {
    manager_->addChat(chat);
  }

  manager_->connectSocket(user.id);
}

void Presenter::setId(int user_id) {
  current_user_id_ = user_id;
  manager_->setCurrentId(user_id);
}

void Presenter::onChatClicked(int chat_id) { openChat(chat_id); }

void Presenter::newMessage(Message& msg) {
  if (msg.senderId == current_user_id_) msg.readed_by_me = true;
  LOG_INFO("New messages for chat {} :({}) with local_id {}", msg.chatId,
           msg.text.toStdString(), msg.local_id.toStdString());

  if (current_chat_id_.has_value() && current_chat_id_ == msg.chatId) {
    int max = message_list_view_->getMaximumMessageScrollBar();
    int value = message_list_view_->getMessageScrollBarValue();
    manager_->addMessageToChat(msg.chatId, msg, false);
    LOG_INFO("In scrollBar max = '{}' and value = '{}'", max, value);
    if (max == value) message_list_view_->scrollToBottom();
  } else {
    manager_->addMessageToChat(msg.chatId, msg, false);
  }
}

void Presenter::findUserRequest(const QString& text) {
  if (text.isEmpty()) {
    manager_->getUserModel()->clear();
    return;
  }

  auto users = manager_->findUsers(text);
  manager_->getUserModel()->clear();

  for (const auto& user : users) {
    if (current_user_id_ != user.id) manager_->getUserModel()->addUser(user);
  }
}

void Presenter::openChat(int chat_id) {  // make unread message = 0; (?)
  PROFILE_SCOPE("Presenter::openChat");
  current_chat_id_ = chat_id;
  message_list_view_->setMessageModel(manager_->getMessageModel(chat_id));
  message_list_view_->scrollToBottom();
  auto chat = manager_->getChat(chat_id);
  view_->setChatWindow(chat);
}

void Presenter::onUserClicked(int user_id, bool is_user) {
  manager_->getUserModel()->clear();
  view_->clearFindUserEdit();

  if (is_user && current_user_id_ == user_id) {
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
  if (text_to_send.isEmpty() || !current_chat_id_) {
    if (text_to_send.isEmpty())
      LOG_WARN("Presenter receive to send empty text");
    else
      onErrorOccurred("Presenter doesn't have opened chat");
    return;
  }

  Message message_to_send{.chatId = *current_chat_id_,
                          .senderId = *current_user_id_,
                          .text = text_to_send,
                          .status_sended = false,
                          .timestamp = QDateTime::currentDateTime(),
                          .local_id = QUuid::createUuid().toString()};
  LOG_INFO("Set for message {} local_id {}", message_to_send.text.toStdString(),
           message_to_send.local_id.toStdString());
  newMessage(message_to_send);
  manager_->sendMessage(message_to_send);
}

void Presenter::onLogOutButtonClicked() { manager_->logout(); }
