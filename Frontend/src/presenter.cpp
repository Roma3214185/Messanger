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

#include "OpenResponceHandler.h"
#include "NewMessageResponceHandler.h"

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

bool checkOpenedChatAndUser(const std::optional<long long>& current_opened_chat_id_, const std::optional<User>& current_user_) {
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
    static Message createMessage(long long chat_id, long long sender_id, const QString& text, const QString& local_id, QDateTime timestamp = QDateTime::currentDateTime()) {
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

  auto token_opt = manager_->checkToken(); //todo: signal and slot on Token finded(??)
  if(token_opt) manager_->session()->authentificatesWithToken(*token_opt);
  initialConnections();
  initialHandlers();
  manager_->setupConnections();
}

void Presenter::initialHandlers() {
  const std::string openedType     = "opened";
  const std::string newMessageType = "new_message";

  socket_responce_handlers_[openedType] = std::make_unique<OpenResponceHandler>(manager_->getTokenManager(), manager_->socket());
  socket_responce_handlers_[openedType] = std::make_unique<NewMessageResponceHandler>(manager_->getTokenManager(), manager_->message());
}

void Presenter::setMessageListView(IMessageListView* message_list_view) {
  message_list_view_ = message_list_view;
  view_->setMessageListView(static_cast<QListView*>(message_list_view));
}

void Presenter::signIn(const LogInRequest& login_request) { manager_->session()->signIn(login_request); }

void Presenter::signUp(const SignUpRequest& req) { manager_->session()->signUp(req); }

void Presenter::initialConnections() {
  connect(manager_->session(), &SessionUseCase::userCreated, this, &Presenter::setUser);
  connect(manager_->socket(), &SocketUseCase::newResponce, this, &Presenter::onNewResponce);
  connect(message_list_view_, &IMessageListView::scrollChanged, this, &Presenter::onScroll);
}

void Presenter::onNewResponce(QJsonObject& json_object) {
  LOG_INFO("Received new Json from socket");
  if(!json_object.contains("type")) {
    LOG_ERROR("In json no 'type' filed");
    return;
  }

  const std::string type           = json_object["type"].toString().toStdString();
  if(socket_responce_handlers_.find(type) != socket_responce_handlers_.end()) {
    socket_responce_handlers_[type]->handle(json_object);
  } else {
    LOG_ERROR("Invalid type {}", type);
  }
}

MessageDelegate* Presenter::getMessageDelegate() {
  if(!message_delegate_) message_delegate_ = std::make_unique<MessageDelegate>(manager_->getDataManager(), manager_->getTokenManager());
  return message_delegate_.get();
}

UserDelegate* Presenter::getUserDelegate() {
  if(!user_delegate_) user_delegate_ = std::make_unique<UserDelegate>();
  return user_delegate_.get();
}

ChatItemDelegate* Presenter::getChatDelegate() {
  if(!chat_delegate_) chat_delegate_ = std::make_unique<ChatItemDelegate>();
  return chat_delegate_.get();
}

void Presenter::onScroll(int value) {
  if(!current_opened_chat_id_) {
    LOG_ERROR("Chat not opened");
    return;
  }

  bool chat_list_is_on_top = (value == 0);
  if (!chat_list_is_on_top) return;

  PROFILE_SCOPE("Presenter::onScroll");
  long long chat_id = *current_opened_chat_id_;
  constexpr int limit_of_loading_messages = 20;
  auto newMessages = manager_->message()->getChatMessages(chat_id, limit_of_loading_messages);
  if (newMessages.empty()) return;

  auto message_model = manager_->getMessageModel(chat_id);
  assert(message_model);

  message_list_view_->preserveFocusWhile(message_model, [&]{
    for (const auto& msg : newMessages) {
      manager_->message()->addMessageToChat(msg); //TODO: make pipeline
    }
  });
  // TODO: think about future / then
}

void Presenter::onErrorOccurred(const QString& error) { view_->showError(error); }

void Presenter::setUser(const User& user, const QString& token) {
  PROFILE_SCOPE("Presenter::setUser");
  debug("In set user:", user);
  if(user.id <= 0) {
    LOG_ERROR("Invalid id in set User"); //todo(roma): make id field with struct that can't be <= 0
    return;
  }

  current_user_ = user;
  manager_->saveData(token, user.id);
  manager_->socket()->connectSocket();
  manager_->chat()->loadChatsAsync();
  manager_->getDataManager()->saveUser(user);
  Q_EMIT userSetted();
}

void Presenter::setCurrentChatId(long long chat_id) {
  assert(chat_id > 0); //todo(roma): implement struct Id to assert every time
  current_opened_chat_id_ = chat_id;
}

void Presenter::onChatClicked(long long chat_id) { openChat(chat_id); }

void Presenter::newMessage(Message& msg) {
  if(!current_user_) {
    LOG_ERROR("User isn't initialized");
    return;
  }

  if (msg.senderId == current_user_->id) msg.readed_by_me = true;
  debug("New message received from socket", msg);

  int max   = message_list_view_->getMaximumMessageScrollBar();
  int value = message_list_view_->getMessageScrollBarValue();

  manager_->message()->addMessageToChat(msg);

  if (current_opened_chat_id_.has_value() && current_opened_chat_id_ == msg.chatId && max == value) {
    message_list_view_->scrollToBottom();
  }
}

void Presenter::findUserRequest(const QString& text) {
  auto user_model = manager_->getUserModel();
  assert(user_model);
  user_model->clear();

  if (text.isEmpty()) {
    return;
  }

  if(!current_user_) {
    LOG_ERROR("User isn't inislized");
    return;
  }

  auto users = manager_->user()->findUsers(text);

  for (const auto& user : users) {
    if (current_user_->id != user.id) manager_->getUserModel()->addUser(user);
  }
}

void Presenter::openChat(long long chat_id) {  // make unread message = 0; (?)
  PROFILE_SCOPE("Presenter::openChat");
  setCurrentChatId(chat_id);
  message_list_view_->setMessageModel(manager_->getMessageModel(chat_id));
  message_list_view_->scrollToBottom();
  auto chat = manager_->chat()->getChat(chat_id);
  view_->setChatWindow(chat);
}

void Presenter::onUserClicked(long long user_id, bool is_user) {
  auto user_model = manager_->getUserModel();
  assert(user_model);
  user_model->clear();
  view_->clearFindUserEdit();

  if(!current_user_) {
    LOG_ERROR("User isn't inislized");
    return;
  }


  if (is_user && current_user_->id == user_id) {
    onErrorOccurred("[ERROR] Impossible to open chat with yourself");
    return;
  }

  if (is_user) {
    auto chat = manager_->chat()->getPrivateChatWithUser(user_id);
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
  manager_->message()->addMessageToChat(message_to_send);
  message_list_view_->scrollToBottom();
  manager_->socket()->sendMessage(message_to_send);
}

void Presenter::onLogOutButtonClicked() {
  manager_->logout();
  current_user_ .reset();
  current_opened_chat_id_.reset();
}
