#include "handlers/SocketHandlerRegistry.h"
#include "JsonService.h"
#include "handlers/Handlers.h"
#include "managers/datamanager.h"
#include "model.h"

SocketHandlerRegistry::SocketHandlersMap SocketHandlerRegistry::create(Model* manager, JsonService* json_service) {
  SocketHandlerRegistry::SocketHandlersMap handlers;

  handlers["opened"] = std::make_unique<OpenSocketHandler>(manager->tokenManager(), manager->socket());

  handlers["new_message"] =
      std::make_unique<NewMessageHandler>(json_service, manager->dataManager(), manager->dataManager());

  handlers["delete_message"] = std::make_unique<DeleteMessageHandler>(json_service, manager->dataManager());

  handlers["read_message"] = std::make_unique<ReadMessageHandler>(json_service, manager->dataManager());

  handlers["save_reaction"] = std::make_unique<SaveMessageReactionHandler>(json_service, manager->dataManager());

  handlers["delete_reaction"] = std::make_unique<DeleteMessageReactionHandler>(json_service, manager->dataManager());

  return handlers;
}
