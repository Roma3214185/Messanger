#include "model.h"

#include <QString>

#include "Debug_profiling.h"
#include "JsonService.h"
#include "dto/SignUpRequest.h"
#include "dto/User.h"
#include "interfaces/ICache.h"
#include "interfaces/INetworkAccessManager.h"
#include "interfaces/ISocket.h"
#include "managers/TokenManager.h"
#include "managers/datamanager.h"
#include "models/UserModel.h"
#include "models/chatmodel.h"
#include "models/messagemodel.h"
#include "usecases/UseCaseRepository.h"

Model::Model(UseCaseRepository *use_case_repostirory, ICache *cash, TokenManager *token_manager, ISocket *socket,
             DataManager *data_manager)
    : use_case_repository_(use_case_repostirory),
      cache_(cash),
      token_manager_(token_manager),
      chat_model_(std::make_unique<ChatModel>()),
      user_model_(std::make_unique<UserModel>()),
      data_manager_(data_manager) {}

void Model::setupConnections() {
  connect(data_manager_, &DataManager::chatAdded, this, [this](const ChatPtr &added_chat) {
    DBC_REQUIRE(added_chat != nullptr);
    chatModel()->addChat(added_chat);
    message()->getChatMessagesAsync(added_chat->chat_id);
  });

  connect(data_manager_, &DataManager::messageAdded, this, [this](const Message &message) {
    user()->getUserAsync(message.sender_id);
    auto last_message = messageModel(message.chat_id)->getLastMessage();
    chat_model_->updateChatInfo(message.chat_id, last_message);
    //  todo: if chat message.chat_id doesn't exist -> load it
  });

  connect(data_manager_, &DataManager::messageDeleted, this, [this](const Message &deleted_message) {
    DBC_REQUIRE(deleted_message.checkInvariants());
    auto *message_model = messageModel(deleted_message.chat_id);
    message_model->deleteMessage(deleted_message);
  });
}

std::optional<QString> Model::checkToken() {
  if (auto tokenOpt = cache_->get("TOKEN"); tokenOpt.has_value()) {
    LOG_INFO("Token found: '{}'", *tokenOpt);
    return QString::fromStdString(*tokenOpt);
  }

  LOG_INFO("No token found");
  return std::nullopt;
}

void Model::saveData(const QString &token, long long current_id) {
  DBC_REQUIRE(!token.isEmpty() && current_id > 0);
  token_manager_->setData(token, current_id);
  cache_->saveToken("TOKEN", token.toStdString());
}

MessageModel *Model::messageModel(long long chat_id) { //todo: dangerous, refactor
  DBC_REQUIRE(chat_id > 0);
  auto message_model = data_manager_->getMessageModel(chat_id);
  DBC_REQUIRE(message_model);
  return message_model.get();
}

void Model::clearAll() {
  PROFILE_SCOPE();
  socket()->close();
  data_manager_->clearAll();
  cache_->deleteToken("TOKEN");
  token_manager_->resetData();
  chat_model_->clear();
  token_manager_->resetData();
}

SessionUseCase *Model::session() const { return use_case_repository_->session(); }
MessageUseCase *Model::message() const { return use_case_repository_->message(); }
UserUseCase *Model::user() const { return use_case_repository_->user(); }
ChatUseCase *Model::chat() const { return use_case_repository_->chat(); }
DataManager *Model::dataManager() const { return data_manager_; }
TokenManager *Model::tokenManager() const { return token_manager_; }
SocketUseCase *Model::socket() const { return use_case_repository_->socket(); }
ChatModel *Model::chatModel() const noexcept { return chat_model_.get(); }
UserModel *Model::userModel() const noexcept { return user_model_.get(); }
