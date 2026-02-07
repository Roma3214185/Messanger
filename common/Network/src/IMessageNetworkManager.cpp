#include "interfaces/IMessageNetworkManager.h"

#include "Debug_profiling.h"
#include "config/codes.h"
#include "config/ports.h"
#include "entities/ReactionInfo.h"
#include "utils.h"
#include "entities/RequestDTO.h"
#include "proxyclient.h"

MessageNetworkManager::MessageNetworkManager(ProxyClient* proxy) : proxy_(proxy) {}

std::optional<long long> MessageNetworkManager::getChatIdOfMessage(long long message_id) {
  const std::string path = "/message/" + std::to_string(message_id);
    RequestDTO request;
  request.path = path;
    request.method = "GET";
  auto [code, body] = proxy_->forward(request, Config::Ports::messageService);


  LOG_INFO("getChatIdOfMessage received {} and body {}", code, body);
  if (code != Config::StatusCodes::success) {
    LOG_ERROR("getChatIdOfMessage failed: {}", code);
    return std::nullopt;
  }

  try {
    const nlohmann::json obj = nlohmann::json::parse(body);

    if (!obj.is_object()) {
      LOG_ERROR("Invalid JSON format in getChatIdOfMessage");
      return std::nullopt;
    }

    if (!obj.contains("chat_id")) {
      LOG_ERROR("Obj doesn't contain 'chat_id' field");
      return std::nullopt;
    }

    long long chat_id = obj.at("chat_id").get<long long>();
    LOG_INFO("For message id {} chat id is {}", message_id, chat_id);
    return chat_id;
  } catch (const std::exception &e) {
    LOG_ERROR("JSON parse error in getChatIdOfMessage: {}", e.what());
    return std::nullopt;
  }
}

std::optional<ReactionInfo> MessageNetworkManager::getReaction(long long reaction_id) {
  const std::string path = "/reaction/" + std::to_string(reaction_id);  // todo: new service
    RequestDTO request;
    request.path = path;
    request.method = "GET";
  auto [code, body] = proxy_->forward(request, Config::Ports::reactionService);
  LOG_INFO("getChatIdOfMessage received {} and body {}", code, body);
  if (code != Config::StatusCodes::success) {
    LOG_ERROR("getChatIdOfMessage failed: {}", code);
    return std::nullopt;
  }

  return utils::parsePayload<ReactionInfo>(body);
}
