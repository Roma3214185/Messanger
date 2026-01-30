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
  connect(data_manager_, &DataManager::messageAdded, this, [&](const Message &added_messaage) {  // todo: not make copy
    LOG_INFO("Received DataManager::messageAdded (text is {})", added_messaage.getFullText().toStdString());
    DBC_REQUIRE(added_messaage.chat_id > 0);
    auto message_model = data_manager_->getMessageModel(added_messaage.chat_id);
    DBC_REQUIRE(message_model);
    message_model->saveMessage(added_messaage);
    if (!added_messaage.isOfflineSaved()) Q_EMIT messageAdded(added_messaage);  // this message from server, not offline
  });

  connect(message_manager_.get(), &MessageManager::saveReactionInfo, this,
          [&](const ReactionInfo &reaction_info) { data_manager_->save(reaction_info); });
}

auto MessageUseCase::getChatMessages(long long chat_id, int limit) -> QList<Message> {
  auto message_model = data_manager_->getMessageModel(chat_id);
  DBC_REQUIRE(message_model != nullptr);
  auto oldestMessage = message_model->getOldestMessage();

  long long id_of_oldest_message = oldestMessage.has_value() ? oldestMessage->id : 0ll;

  // TODO(roma): cache request result for {chat_id before_id}
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

  // todo: MessageUseCase seems as work with server, so or clearAllMessages in also must send on server, or
  // refactor/move function
}

void MessageUseCase::getChatMessagesAsync(long long chat_id) {
  PROFILE_SCOPE("MessageUseCase::getChatMessagesAsync");
  DBC_REQUIRE(chat_id > 0);

  QtConcurrent::run([this, chat_id]() { return getChatMessages(chat_id); })
      .then(this,
            [this, chat_id](const QList<Message> &chat_messages) {
              LOG_INFO("[getChatMessagesAsync] For chat '{}' loaded '{}' messages", chat_id, chat_messages.size());
              data_manager_->save(chat_messages);
            })
      .onFailed(this, [chat_id]() { LOG_ERROR("Error in getChatMessagesAsync for chat_id {}", chat_id); });
}
