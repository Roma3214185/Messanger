#include "handlers/Handlers.h"

DeleteMessageReactionHandler::DeleteMessageReactionHandler(EntityFactory *entity_factory, DataManager *data_manager)
    : entity_factory_(entity_factory), data_manager_(data_manager) {}

void DeleteMessageReactionHandler::handle(const QJsonObject &json_object) {
  auto reaction = entity_factory_->getReaction(json_object);  // entity_factory_->get<Reaction>(json_object);
  data_manager_->deleteReaction(reaction);
}

NewMessageResponceHandler::NewMessageResponceHandler(EntityFactory *entity_factory, DataManager *data_manager)
    : entity_factory_(entity_factory), data_manager_(data_manager) {}

void NewMessageResponceHandler::handle(const QJsonObject &json_object) {
  auto [message, reactions] = entity_factory_->getMessageFromJson(json_object);
  data_manager_->save(message);
  data_manager_->save(reactions);
}

OpenResponceHandler::OpenResponceHandler(TokenManager *token_manager, SocketUseCase *socket_use_case)
    : token_manager_(token_manager), socket_use_case_(socket_use_case) {}

void OpenResponceHandler::handle([[maybe_unused]] const QJsonObject &json_object) {
  const long long id = token_manager_->getCurrentUserId();
  socket_use_case_->initSocket(id);
}

ReadMessageHandler::ReadMessageHandler(DataManager *data_manager) : data_manager_(data_manager) {}

void ReadMessageHandler::handle(const QJsonObject &json_object) {
  if (!json_object.contains("message_id")) {
    LOG_ERROR("ReadMessageHandler doen't have field message_id");
    return;
  }

  if (!json_object.contains("receiver_id")) {
    LOG_ERROR("ReadMessageHandler doen't have field receiver_id");
    return;
  }

  long long message_id = json_object["message_id"].toInteger();
  long long readed_by = json_object["receiver_id"].toInteger();
  data_manager_->readMessage(message_id, readed_by);
}

SaveMessageReactionHandler::SaveMessageReactionHandler(EntityFactory *entity_factory, DataManager *data_manager)
    : entity_factory_(entity_factory), data_manager_(data_manager) {}

void SaveMessageReactionHandler::handle(const QJsonObject &json_object) {
  auto reaction = entity_factory_->getReaction(json_object);
  data_manager_->save(reaction);
}
