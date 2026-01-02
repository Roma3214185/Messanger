#ifndef IMESSAGENETWORKMANAGER_H
#define IMESSAGENETWORKMANAGER_H

#include "interfaces/INetworkManagerBase.h"
#include "Debug_profiling.h"
#include <nlohmann/json.hpp>

class IMessageNetworkManager : public virtual INetworkManagerBase {
  public:
    std::optional<long long> getChatIdOfMessage(long long message_id) {
      const std::string path = "/message/" + std::to_string(message_id);
      auto res = forward(provider_->ports().messageService, "", path, "GET");
      LOG_INFO("getChatIdOfMessage reveived {} and body {}", res.first, res.second);
      if (res.first != provider_->statusCodes().success) {
        LOG_ERROR("getChatIdOfMessage failed: {}", res.first);
        return std::nullopt;
      }

      try {
        const nlohmann::json obj = nlohmann::json::parse(res.second);

        if (!obj.is_object()) {
          LOG_ERROR("Invalid JSON format in getChatIdOfMessage");
          return std::nullopt;
        }

        if(!obj.contains("chat_id")) {
          LOG_ERROR("Obj doesn't contain 'id' field");
          return std::nullopt;
        }

        long long chat_id = obj.at("chat_id").get<long long>();
        LOG_INFO("For message id {} chat id is {}", message_id, chat_id);
        return chat_id;
      }
      catch (const std::exception& e) {
        LOG_ERROR("JSON parse error in getChatIdOfMessage: {}", e.what());
        return std::nullopt;
      }
    }
};

#endif // IMESSAGENETWORKMANAGER_H
