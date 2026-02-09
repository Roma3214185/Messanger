#include "presenter.h"

#include <QJsonObject>
#include <QTextDocument>
#include <QtConcurrent/QtConcurrent>
#include <iostream>
#include <nlohmann/json.hpp>

#include "DataInputService.h"
#include "Debug_profiling.h"
#include "JsonService.h"
#include "dto/SignUpRequest.h"
#include "dto/User.h"
#include "entities/MessageStatus.h"
#include "entities/Reaction.h"
#include "handlers/Handlers.h"
#include "interfaces/IMainWindow.h"
#include "ui/MessageListView.h"
#include "managers/Managers.h"
#include "managers/TokenManager.h"
#include "model.h"
#include "models/messagemodel.h"
#include "ui/MessageListView.h"
#include "usecases/chatusecase.h"
#include "usecases/messageusecase.h"
#include "usecases/sessionusecase.h"
#include "usecases/socketusecase.h"
#include "usecases/userusecase.h"
#include "utils.h"

Presenter::Presenter(IMainWindow *window, Model *manager) : view_(window), manager_(manager) {}

void Presenter::initialise() {
  view_->setChatModel(manager_->getChatModel());
  view_->setUserModel(manager_->getUserModel());

  initialConnections();
  manager_->setupConnections();

  if (auto token_opt = manager_->checkToken(); token_opt.has_value()) {
    manager_->session()->authentificatesWithToken(token_opt.value());  // todo: signal and slot on Token finded(??)
  }
}

void Presenter::initialHandlers(SocketHandlersMap handlers) { socket_responce_handlers_ = std::move(handlers); }

void Presenter::setMessageListView(MessageListView *message_list_view) {
  DBC_REQUIRE(message_list_view != nullptr);
  message_list_view_ = message_list_view;
  connect(message_list_view_, &MessageListView::scrollChanged, this, &Presenter::onChatWidgetScroll);
}

void Presenter::signIn(const LogInRequest &login_request) {
  auto res = DataInputService::validateLoginUserInput(login_request);
  if (!res.valid) {
    showError(res.message);
  } else {
    manager_->session()->signIn(login_request);
  }
}

void Presenter::signUp(const SignUpRequest &signup_request) {
  auto res = DataInputService::validateRegistrationUserInput(signup_request);
  if (!res.valid) {
    showError(res.message);
  } else {
    manager_->session()->signUp(signup_request);
  }
}

void Presenter::showError(const QString &error) { view_->showError(error); }

void Presenter::initialConnections() {
  connect(manager_->session(), &ISessionUseCase::userCreated, this, &Presenter::setUser);
  connect(manager_->socket(), &ISocketUseCase::newResponce, this, &Presenter::onNewSocketMessage);
}

void Presenter::onNewSocketMessage(const QJsonObject &socket_message) {
  LOG_INFO("Received new Json from socket");
  if (!socket_message.contains("type")) {
    LOG_ERROR("In json no 'type' filed");
    return;
  }

  const auto type = socket_message["type"].toString().toStdString();
  if (socket_responce_handlers_.contains(type)) {
    socket_responce_handlers_[type]->handle(socket_message);
  } else {
    LOG_ERROR("Invalid type {}", type);
  }
}

void Presenter::deleteMessage(const Message &message) { manager_->message()->deleteMessage(message); }

void Presenter::reactionClicked(const Message &message, long long reaction_id) {
  LOG_INFO("Make reaction {} id for message {}", reaction_id, message.toString());
  DBC_REQUIRE(!message.isOfflineSaved());

  const long long current_user_id = manager_->tokenManager()->getCurrentUserId();
  Reaction new_reaction{message.id, current_user_id, reaction_id};

  if (bool there_was_not_reaction = !message.receiver_reaction.has_value(); there_was_not_reaction) {
    saveReaction(new_reaction);
  } else if (utils::isSame(message.receiver_reaction, reaction_id)) {
    LOG_INFO("Clicked on reaction that already setted, need to just delete current");
    deleteReaction(new_reaction);
  } else {
    LOG_INFO("User changed reaction, from {} to {}", message.receiver_reaction.value(), reaction_id);
    Reaction old_reaction{message.id, current_user_id, message.receiver_reaction.value()};
    deleteReaction(old_reaction);
    saveReaction(new_reaction);
  }
}

void Presenter::saveReaction(const Reaction &reaction) {
  manager_->dataManager()->save(reaction);
  manager_->socket()->saveReaction(reaction);
}

void Presenter::deleteReaction(const Reaction &reaction) {
  manager_->dataManager()->deleteReaction(reaction);
  manager_->socket()->deleteReaction(reaction);
}

void Presenter::onChatWidgetScroll(int distance_from_top) {
  if (!current_opened_chat_id_.has_value()) return;
  if (distance_from_top == 0) return;
  PROFILE_SCOPE();

  constexpr int kLimitOfLoadingMessages = 20;
  auto new_messages = manager_->message()->getChatMessages(*current_opened_chat_id_, kLimitOfLoadingMessages);
  if (new_messages.empty()) return;

  auto *message_model = manager_->getMessageModel(*current_opened_chat_id_);
  DBC_REQUIRE(message_model);

  message_list_view_->preserveFocusWhile(message_model, [&] { manager_->dataManager()->save(new_messages); });
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
  Q_EMIT userSetted();
}

void Presenter::setCurrentChatId(long long chat_id) {
  DBC_REQUIRE(chat_id > 0);
  current_opened_chat_id_ = chat_id;
}

void Presenter::onChatClicked(long long chat_id) { openChat(chat_id); }

void Presenter::onMessageReceivedFromSocket(const Message &msg) {
  DBC_REQUIRE(current_user_ != std::nullopt);
  LOG_INFO("New message received from socket {}", msg.toString());

  const int max = message_list_view_->getMaximumMessageScrollBar();
  const int value = message_list_view_->getMessageScrollBarValue();
  manager_->dataManager()->save(msg);
  if (current_opened_chat_id_.has_value() && current_opened_chat_id_ == msg.chat_id && max == value) {
    message_list_view_->scrollToBottom();
  }
}

void Presenter::findUserRequest(const QString &text) {
  if (text.isEmpty()) return;
  DBC_REQUIRE(current_user_ != std::nullopt);
  auto users = manager_->user()->findUsers(text);
  auto *user_model = manager_->getUserModel();
  user_model->clear();
  for (const auto &user : users) {
    if (current_user_->id != user.id) manager_->getUserModel()->addUser(user);
  }
}

void Presenter::openChat(long long chat_id) {
  PROFILE_SCOPE();
  DBC_REQUIRE(chat_id > 0);
  setCurrentChatId(chat_id);
  message_list_view_->setMessageModel(manager_->getMessageModel(chat_id));
  message_list_view_->scrollToBottom();  // todo: not in every sitation it's good idea

  if (auto chat = manager_->dataManager()->getChat(chat_id); chat != nullptr) {
    view_->setChatWindow(chat);
  } else {
    LOG_ERROR("Chat to open not found");  // todo: in this case request on server
  }
}

void Presenter::onUserClicked(long long user_id, bool is_user) {
  DBC_REQUIRE(user_id > 0);
  DBC_REQUIRE(current_user_ != std::nullopt);
  auto *user_model = manager_->getUserModel();
  user_model->clear();

  if (is_user && current_user_->id == user_id) {
    onErrorOccurred("Impossible to open chat with yourself");
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

void Presenter::sendButtonClicked(QTextDocument *document_with_text_to_send,
                                  std::optional<long long> answer_on_message_id) {
  DBC_REQUIRE(current_opened_chat_id_ != std::nullopt);
  DBC_REQUIRE(current_user_ != std::nullopt);
  DBC_REQUIRE(document_with_text_to_send != nullptr);

  if (!current_opened_chat_id_ || !current_user_) {
    return;
  }

  if (document_with_text_to_send->isEmpty()) {
    LOG_WARN("Presenter receive to send empty text");
    return;
  }

  auto tokens = utils::text::get_tokens_from_doc(document_with_text_to_send);
  if (tokens.empty()) {
    LOG_WARN("tokens to send empty text");
    return;
  }

  auto message_to_send =
      MessageFactory::createMessage(manager_->tokenManager()->getCurrentUserId(), *current_opened_chat_id_,
                                    current_user_->id, tokens, QUuid::createUuid().toString(), answer_on_message_id);
  LOG_INFO("Message to send {}", message_to_send.toString());
  manager_->dataManager()->save(message_to_send);
  message_list_view_->scrollToBottom();
  manager_->socket()->sendMessage(message_to_send);  // todo(?): implement sending message via HHTP, not socket
}

void Presenter::onLogOutButtonClicked() {
  manager_->clearAll();
  current_user_.reset();
  current_opened_chat_id_.reset();
}

std::optional<ReactionInfo> Presenter::getReactionInfo(long long reaction_id) {
  return manager_->dataManager()->getReactionInfo(reaction_id);
}

std::vector<Message> Presenter::getListOfMessagesBySearch(const QString &prefix_to_seach_by) {
  DBC_REQUIRE(current_opened_chat_id_ != std::nullopt);

  QString prefix_trimmed = prefix_to_seach_by.trimmed();
  if (prefix_trimmed.isEmpty()) {
    return {};
  }

  auto list_of_messages_of_chat = manager_->dataManager()->getMessageModel(*current_opened_chat_id_)->messages();
  auto ans = std::vector<Message>{};
  for (const auto &message : list_of_messages_of_chat) {
    if (QString text = message.getPlainText(); text.contains(prefix_trimmed)) {
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

  MessageStatus status;
  status.message_id = message.id;
  status.receiver_id = current_user_id;
  status.is_read = true;
  // todo: status.read_at = or factory
  manager_->dataManager()->save(status);
  manager_->socket()->sendReadMessageEvent(status);
}

std::vector<ReactionInfo> Presenter::getReactionsForMenu() { return manager_->dataManager()->getEmojiesForMenu(); }

void Presenter::editMessage(Message &message_to_edit, QTextDocument *edited_text) {
  DBC_REQUIRE(current_opened_chat_id_ != std::nullopt);
  DBC_REQUIRE(current_user_ != std::nullopt);
  if (!edited_text) {
    LOG_ERROR("Presenter nullptr QTextDocument");
    return;
  }

  if (edited_text->isEmpty()) {
    LOG_WARN("Presenter receive to send empty doc");
    return;
  }

  auto tokens = utils::text::get_tokens_from_doc(edited_text);

  if (tokens.empty()) {
    LOG_WARN("tokens to editMessage message is empty");
    return;
  }

  if (message_to_edit.tokens == tokens) {
    LOG_WARN("tokens to editMessage message is same as before, skip update");
    return;
  }

  message_to_edit.tokens = tokens;

  manager_->dataManager()->save(message_to_edit);
  manager_->socket()->sendMessage(message_to_edit);
}
