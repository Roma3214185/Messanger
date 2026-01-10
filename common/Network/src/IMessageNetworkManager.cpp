#include "interfaces/IMessageNetworkManager.h"

#include "Debug_profiling.h"
#include "config/codes.h"
#include "config/ports.h"

std::optional<long long> IMessageNetworkManager::getChatIdOfMessage(long long message_id) {
  const std::string path = "/message/" + std::to_string(message_id);
  auto [code, body] = forward(Config::Ports::messageService, "", path, "GET");
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
