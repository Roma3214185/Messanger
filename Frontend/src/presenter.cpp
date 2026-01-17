#include "presenter.h"

#include <QJsonObject>
#include <QtConcurrent/QtConcurrent>
#include <iostream>
#include <nlohmann/json.hpp>
#include <QTextDocument>

#include "Debug_profiling.h"
#include "DeleteMessageResponce.h"
#include "JsonService.h"
#include "MessageListView.h"
#include "Utils.h"
#include "dto/SignUpRequest.h"
#include "dto/User.h"
#include "entities/Reaction.h"
#include "handlers/Handlers.h"
#include "interfaces/IMainWindow.h"
#include "interfaces/IMessageListView.h"
#include "model.h"
#include "models/messagemodel.h"
#include "utils.h"

Presenter::Presenter(IMainWindow *window, Model *manager) : view_(window), manager_(manager) {}

void Presenter::initialise() {
  view_->setChatModel(manager_->getChatModel());
  view_->setUserModel(manager_->getUserModel());

  initialConnections();
  initialHandlers();
  manager_->setupConnections();

  if (auto token_opt = manager_->checkToken(); token_opt.has_value()) {
    manager_->session()->authentificatesWithToken(token_opt.value());  // todo: signal and slot on Token finded(??)
  }
}

void Presenter::initialHandlers() {
  const std::string opened_type = "opened";
  const std::string new_message_type = "new_message";
  const std::string delete_message_type = "delete_message";
  const std::string read_message_type = "read_message";
  const std::string save_reaction_type = "save_reaction";
  const std::string delete_reaction_type = "delete_reaction";

  socket_responce_handlers_[opened_type] =
      std::make_unique<OpenResponceHandler>(manager_->tokenManager(), manager_->socket());
  socket_responce_handlers_[new_message_type] =
      std::make_unique<NewMessageResponceHandler>(manager_->entities(), manager_->dataManager());
  socket_responce_handlers_[delete_message_type] =
      std::make_unique<DeleteMessageResponceHandler>(manager_->entities(), manager_->message());
  socket_responce_handlers_[read_message_type] = std::make_unique<ReadMessageHandler>(manager_->dataManager());
  socket_responce_handlers_[save_reaction_type] =
      std::make_unique<SaveMessageReactionHandler>(manager_->entities(), manager_->dataManager());
  socket_responce_handlers_[delete_reaction_type] =
      std::make_unique<DeleteMessageReactionHandler>(manager_->entities(), manager_->dataManager());
}

void Presenter::setMessageListView(IMessageListView *message_list_view) {
  DBC_REQUIRE(message_list_view != nullptr);
  message_list_view_ = message_list_view;
  view_->setMessageListView(static_cast<QListView *>(message_list_view));
}

void Presenter::signIn(const LogInRequest &login_request) { manager_->session()->signIn(login_request); }

void Presenter::signUp(const SignUpRequest &req) { manager_->session()->signUp(req); }

void Presenter::initialConnections() {
  connect(manager_->session(), &SessionUseCase::userCreated, this, &Presenter::setUser);
  connect(manager_->socket(), &SocketUseCase::newResponce, this, &Presenter::onNewResponce);
  connect(message_list_view_, &IMessageListView::scrollChanged, this, &Presenter::onScroll);
  connect(message_delegate_.get(), &MessageDelegate::unreadMessage, this, &Presenter::onUnreadMessage);
}

void Presenter::onNewResponce(QJsonObject &json_object) {
  LOG_INFO("Received new Json from socket");
  if (!json_object.contains("type")) {
    LOG_ERROR("In json no 'type' filed");
    return;
  }

  const std::string type = json_object["type"].toString().toStdString();
  if (socket_responce_handlers_.contains(type)) {
    socket_responce_handlers_[type]->handle(json_object);
  } else {
    LOG_ERROR("Invalid type {}", type);
  }
}

MessageDelegate *Presenter::getMessageDelegate() {
  if (!message_delegate_)
    message_delegate_ = std::make_unique<MessageDelegate>(manager_->dataManager(), manager_->tokenManager());
  return message_delegate_.get();
}

UserDelegate *Presenter::getUserDelegate() {
  if (!user_delegate_) user_delegate_ = std::make_unique<UserDelegate>();
  return user_delegate_.get();
}

ChatItemDelegate *Presenter::getChatDelegate() {
  if (!chat_delegate_) chat_delegate_ = std::make_unique<ChatItemDelegate>(manager_->dataManager());
  return chat_delegate_.get();
}

void Presenter::deleteMessage(const Message &message) {
  DBC_REQUIRE(message.checkInvariants());
  manager_->message()->deleteMessage(message);
}

void Presenter::reactionClicked(const Message &message, long long reaction_id) {
  LOG_INFO("Make reaction {} id for message {}", reaction_id, message.toString());
  DBC_REQUIRE(message.checkInvariants());
  DBC_REQUIRE(!message.isOfflineSaved());
  if (message.isOfflineSaved()) return;  // temporary while contracts don't throw exceptions

  long long current_user_id = manager_->tokenManager()->getCurrentUserId();
  Reaction new_reaction(message.id, current_user_id, reaction_id);

  if (bool there_was_not_reaction = !message.receiver_reaction.has_value(); there_was_not_reaction) {
    saveReaction(new_reaction);
  } else if (utils::isSame(message.receiver_reaction, reaction_id)) {
    LOG_INFO("Clicked on reaction that already setted, need to just delete current");
    deleteReaction(new_reaction);
  } else {
    LOG_INFO("User changed reaction, from {} to {}", message.receiver_reaction.value(), reaction_id);
    Reaction old_reaction(message.id, current_user_id, message.receiver_reaction.value());
    deleteReaction(old_reaction);
    saveReaction(new_reaction);
  }
}

void Presenter::saveReaction(const Reaction &reaction) {
  manager_->socket()->saveReaction(reaction);  // it faster and async than save (?)
  manager_->dataManager()->save(reaction);
}

void Presenter::deleteReaction(const Reaction &reaction) {
  manager_->socket()->deleteReaction(reaction);
  manager_->dataManager()->deleteReaction(reaction);
}

void Presenter::onScroll(int value) {  // todo: multithreaded event changed
                                       // current_opened_chat_id_ (?)
  DBC_REQUIRE(current_opened_chat_id_ != std::nullopt);
  if (bool chat_list_is_on_top = (value == 0); !chat_list_is_on_top) return;

  PROFILE_SCOPE("Presenter::onScroll");
  const long long chat_id = *current_opened_chat_id_;
  constexpr int kLimitOfLoadingMessages = 20;
  auto new_messages = manager_->message()->getChatMessages(chat_id, kLimitOfLoadingMessages);
  if (new_messages.empty()) return;

  auto *message_model = manager_->getMessageModel(chat_id);
  DBC_REQUIRE(message_model);

  message_list_view_->preserveFocusWhile(message_model, [&] {
    for (auto &msg : new_messages) {
      manager_->dataManager()->save(msg);  // TODO: make pipeline
    }
  });
  // TODO: think about future / then
}

void Presenter::onErrorOccurred(const QString &error) {
  DBC_REQUIRE(!error.isEmpty());
  view_->showError(error);
}

void Presenter::setUser(const User &user, const QString &token) {
  PROFILE_SCOPE();
  LOG_INFO("In set user: {} and token {}", user.toString(), token.toStdString());
  DBC_REQUIRE(user.checkInvariants());
  DBC_REQUIRE(!token.isEmpty());

  current_user_ = user;
  manager_->saveData(token, user.id);
  manager_->socket()->connectSocket();
  manager_->chat()->loadChatsAsync();
  manager_->dataManager()->save(user);
  DBC_ENSURE(current_user_ != std::nullopt);
  Q_EMIT userSetted();
}

void Presenter::setCurrentChatId(long long chat_id) {
  DBC_REQUIRE(chat_id > 0);
  current_opened_chat_id_ = chat_id;
}

void Presenter::onChatClicked(long long chat_id) { openChat(chat_id); }

void Presenter::newMessage(Message &msg) {
  DBC_REQUIRE(current_user_ != std::nullopt);

  if (msg.isMine()) {
    qDebug() << msg.receiver_read_status;  // todo: can be already always true, refactor to get const Message&
    msg.receiver_read_status = true;
  }
  LOG_INFO("New message received from socket {}", msg.toString());

  const int max = message_list_view_->getMaximumMessageScrollBar();
  const int value = message_list_view_->getMessageScrollBarValue();

  manager_->dataManager()->save(msg);

  if (current_opened_chat_id_.has_value() && current_opened_chat_id_ == msg.chat_id && max == value) {
    message_list_view_->scrollToBottom();
  }
}

void Presenter::findUserRequest(const QString &text) {
  DBC_REQUIRE(!text.isEmpty());
  auto *user_model = manager_->getUserModel();
  DBC_REQUIRE(user_model);
  user_model->clear();

  DBC_REQUIRE(current_user_ != std::nullopt);
  auto users = manager_->user()->findUsers(text);

  for (const auto &user : users) {
    if (current_user_->id != user.id) manager_->getUserModel()->addUser(user);
  }
}

void Presenter::openChat(long long chat_id) {  // make unread message = 0; (?)
  PROFILE_SCOPE("Presenter::openChat");
  DBC_REQUIRE(chat_id > 0);
  setCurrentChatId(chat_id);
  message_list_view_->setMessageModel(manager_->getMessageModel(chat_id));
  message_list_view_->scrollToBottom();  // todo: not in every sitation it's good idea
  view_->setChatWindow(manager_->chat()->getChat(chat_id));
}

void Presenter::onUserClicked(long long user_id, bool is_user) {
  DBC_REQUIRE(user_id > 0);
  DBC_REQUIRE(current_user_ != std::nullopt);
  auto *user_model = manager_->getUserModel();
  DBC_REQUIRE(user_model);
  user_model->clear();
  view_->clearFindUserEdit();  // todo: this should be in mainwindow?

  if (is_user && current_user_->id == user_id) {
    onErrorOccurred("[ERROR] Impossible to open chat with yourself");
    return;
  }

  if (is_user) {
    auto chat = manager_->chat()->getPrivateChatWithUser(user_id);
    DBC_REQUIRE(chat != nullptr);
    openChat(chat->chat_id);
  } else {
    DBC_UNREACHABLE();
    qDebug() << "[ERROR] Implement finding group request";
  }
}

void Presenter::sendButtonClicked(QTextDocument* doc) {
  DBC_REQUIRE(current_opened_chat_id_ != std::nullopt);
  DBC_REQUIRE(current_user_ != std::nullopt);
  DBC_REQUIRE(doc != nullptr);
  // TODO: what if multithreaded will make here current_user is nullopt, after checking (?)

  if (doc->isEmpty()) {
    LOG_WARN("Presenter receive to send empty text");
    return;
  }

  auto tokens = utils::text::get_tokens_from_doc(doc);
  //QString tokensize_text = utils::text::tokenize(tokens);

  if (tokens.empty()) {
    LOG_WARN("tokens to send empty text");
    return;
  }

  auto message_to_send = manager_->entities()->createMessage(*current_opened_chat_id_, current_user_->id, tokens,
                                                             QUuid::createUuid().toString());
  LOG_INFO("Message to send {}", message_to_send.toString());
  manager_->dataManager()->save(message_to_send);
  message_list_view_->scrollToBottom();
  manager_->socket()->sendMessage(message_to_send);  // todo: implement sending message via HHTP, not socket
}

void Presenter::onLogOutButtonClicked() {
  manager_->logout();
  current_user_.reset();
  current_opened_chat_id_.reset();
}
std::optional<ReactionInfo> Presenter::getReactionInfo(long long reaction_id) {
  return manager_->dataManager()->getReactionInfo(reaction_id);
}

std::vector<Message> Presenter::getListOfMessagesBySearch(const QString &prefix) {
  DBC_REQUIRE(current_opened_chat_id_ != std::nullopt);
  QString prefix_trimmed = prefix.trimmed();
  if (prefix_trimmed.isEmpty()) {
    return {};
  }

  auto list_of_messages_of_chat = manager_->dataManager()->getMessageModel(*current_opened_chat_id_)->messages();

  auto ans = std::vector<Message>{};
  for (const auto &message : list_of_messages_of_chat) {
    if(QString text = message.getPlainText(); text.contains(prefix_trimmed)) {
      ans.push_back(message);
    }
  }

  return ans;
}

std::vector<ReactionInfo> Presenter::getDefaultReactionsInChat(long long chat_id) {
  if (auto chat = manager_->dataManager()->getChat(chat_id); chat != nullptr) {
    return chat->default_reactions;
  }
  DBC_UNREACHABLE();
  return {};
}

void Presenter::onUnreadMessage(Message &message) {
  if (message.receiver_read_status == true) {
    DBC_UNREACHABLE();
    return;
  }
  if (message.isOfflineSaved()) return;

  long long current_user_id = manager_->tokenManager()->getCurrentUserId();
  manager_->dataManager()->readMessage(message.id, current_user_id);
  manager_->socket()->sendReadMessageEvent(message, current_user_id);
}

std::vector<ReactionInfo> Presenter::getReactionsForMenu() { return manager_->dataManager()->getEmojiesForMenu(); }

void Presenter::editMessage(Message& message_to_edit, QTextDocument* doc) {
  DBC_REQUIRE(current_opened_chat_id_ != std::nullopt);
  DBC_REQUIRE(current_user_ != std::nullopt);
  DBC_REQUIRE(doc != nullptr);

  if (doc->isEmpty()) {
    LOG_WARN("Presenter receive to send empty doc");
    return;
  }

  auto tokens = utils::text::get_tokens_from_doc(doc);

  if (tokens.empty()) {
    LOG_WARN("tokens to editMessage message is empty");
    return;
  }

  if(message_to_edit.tokens == tokens) {
    LOG_WARN("tokens to editMessage message is same as before, skip update");
    return;
  }

  message_to_edit.tokens = tokens;

  manager_->dataManager()->save(message_to_edit);
  manager_->socket()->sendMessage(message_to_edit);
}
