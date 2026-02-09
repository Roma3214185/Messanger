#include "handlers/Handlers.h"
#include "JsonService.h"
#include "entities/MessageStatus.h"
#include "managers/TokenManager.h"
#include "managers/datamanager.h"
#include "usecases/socketusecase.h"

DeleteMessageReactionHandler::DeleteMessageReactionHandler(IReactionJsonService *entity_factory,
                                                           IReactionDataManager *data_manager)
    : entity_factory_(entity_factory), data_manager_(data_manager) {}

void DeleteMessageReactionHandler::handle(const QJsonObject &json_object) {
  auto reaction = entity_factory_->getReaction(json_object);  // entity_factory_->get<Reaction>(json_object);
  data_manager_->deleteReaction(reaction);
}

DeleteMessageHandler::DeleteMessageHandler(IMessageJsonService *entity_factory,
                                           IMessageDataManager *message_data_manager)
    : entity_factory_(entity_factory), message_data_manager_(message_data_manager) {}

void DeleteMessageHandler::handle(const QJsonObject &json_object) {
  auto [message, reactions] = entity_factory_->getMessageFromJson(json_object);
  message_data_manager_->deleteMessage(message);
}

NewMessageHandler::NewMessageHandler(IMessageJsonService *entity_factory, IMessageDataManager *data_manager,
                                     IReactionDataManager *reaction_data_manager)
    : entity_factory_(entity_factory),
      message_data_manager_(data_manager),
      reaction_data_manager_(reaction_data_manager) {}

void NewMessageHandler::handle(const QJsonObject &json_object) {
  auto [message, reactions] = entity_factory_->getMessageFromJson(json_object);
  message_data_manager_->save(message);
  for (const auto &reaction : reactions) {
    reaction_data_manager_->save(reaction);
  }
}

OpenSocketHandler::OpenSocketHandler(TokenManager *token_manager, SocketUseCase *socket_use_case)
    : token_manager_(token_manager), socket_use_case_(socket_use_case) {}

void OpenSocketHandler::handle([[maybe_unused]] const QJsonObject &json_object) {
  const long long id = token_manager_->getCurrentUserId();
  socket_use_case_->initSocket(id);
}

ReadMessageHandler::ReadMessageHandler(IMessageStatusJsonService *json_service, IMessageStatusDataManager *data_manager)
    : data_manager_(data_manager), json_service_(json_service) {}

void ReadMessageHandler::handle(const QJsonObject &json_object) {
  auto read_status = json_service_->getMessageStatus(json_object);
  if (read_status.has_value()) {
    data_manager_->save(read_status.value());
  } else {
    LOG_ERROR("Invalid read_status");
  }
}

SaveMessageReactionHandler::SaveMessageReactionHandler(IReactionJsonService *entity_factory,
                                                       IReactionDataManager *data_manager)
    : entity_factory_(entity_factory), data_manager_(data_manager) {}

void SaveMessageReactionHandler::handle(const QJsonObject &json_object) {
  auto reaction = entity_factory_->getReaction(json_object);
  data_manager_->save(reaction);
}
