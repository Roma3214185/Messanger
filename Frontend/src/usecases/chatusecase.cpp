#include "usecases/chatusecase.h"

#include <QFuture>
#include <QFutureWatcher>

#include "managers/chatmanager.h"
#include "managers/datamanager.h"
#include "models/chatmodel.h"
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

ChatUseCase::ChatUseCase(ChatManager* chat_manager, DataManager* data_manager, ChatModel* chat_model, TokenManager* token_manager)
    : chat_manager_(chat_manager)
    , data_manager_(data_manager)
    , chat_model_(chat_model)
    , token_manager_(token_manager) {}

auto ChatUseCase::indexByChatId(long long chat_id) -> QModelIndex {
  std::optional<int> idx = chat_model_->findIndexByChatId(chat_id);
  if (!idx.has_value()) {
    LOG_ERROR("ChatUseCase::indexByChatId — chatId '{}' not found", chat_id);
    return {};
  }
  return chat_model_->index(*idx);
}


ChatPtr ChatUseCase::loadChat(long long chat_id) {
  auto future = chat_manager_->loadChat(token_manager_->getToken(), chat_id);
  auto chat = waitForFuture(future);
  if(chat) Q_EMIT chatLoaded(chat->chat_id);
  return chat;
}

auto ChatUseCase::getPrivateChatWithUser(long long user_id) -> ChatPtr {
  PROFILE_SCOPE("ChatUseCase::getPrivateChatWithUser");
  auto chat_ptr = data_manager_->getPrivateChatWithUser(user_id);
  if (chat_ptr) return chat_ptr;

  LOG_INFO("Private chat for this user '{}' not found", user_id);
  auto chat = createPrivateChat(user_id);
  LOG_INFO("Private chat for this user '{}' is created, id '{}'", chat->chat_id, user_id);
  addChatInFront(chat);  // (!) emit chatAdded -> load chat history if exist
  return chat;
}

auto ChatUseCase::createPrivateChat(long long user_id) -> ChatPtr {
  auto future = chat_manager_->createPrivateChat(token_manager_->getToken(), user_id);
  return waitForFuture(future); //todo: create but not saved now (??)
}

auto ChatUseCase::loadChat() -> QList<ChatPtr> {
  auto future = chat_manager_->loadChats(token_manager_->getToken());
  return waitForFuture(future);
}

void ChatUseCase::addChatInFront(const ChatPtr& chat) {
  PROFILE_SCOPE("ChatUseCase::addChatInFront");
  addChat(chat);
  chat_model_->realocateChatInFront(chat->chat_id); //todo: delete this, sort instead
}

auto ChatUseCase::getNumberOfExistingChats() const -> int {
  int size = data_manager_->getNumberOfExistingChats();
  LOG_INFO("[getNumberOfExistingChats] Number of chats={}", size);
  return size;
}

void ChatUseCase::logout() {
  PROFILE_SCOPE("ChatUseCase::logout");
  LOG_INFO("[logout] Logging out");
  clearAllChats();
  chat_model_->clear();
  LOG_INFO("[logout] Logout chat use case is completed");
}

void ChatUseCase::clearAllChats() {
  data_manager_->clearAllChats();
  LOG_INFO("[clearAllChats] clearAllChats complete");
}

void ChatUseCase::addChat(const ChatPtr& chat) {
  PROFILE_SCOPE("ChatUseCase::addChat");
  data_manager_->addChat(chat);
  chat_model_->addChat(chat);
  Q_EMIT chatAdded(chat->chat_id);
}

ChatPtr ChatUseCase::getChat(long long chat_id) { return data_manager_->getChat(chat_id); }

void ChatUseCase::createChat(long long chat_id) {
  PROFILE_SCOPE("ChatUseCase::createChat");
  auto chat = data_manager_->getChat(chat_id);
  if (chat) {
    LOG_INFO("[Chat '{}' already exist", chat_id);
    return;
  }
  auto new_chat = loadChat(chat_id);
  //message_use_case->fillChatWithMessages(chat_id);
  chat_model_->addChatInFront(new_chat); //todo: make just add, and in chat_model_ sort
}
