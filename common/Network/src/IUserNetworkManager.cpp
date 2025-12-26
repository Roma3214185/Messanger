#include "interfaces/IUserNetworkManager.h"

#include "entities/User.h"
#include "Debug_profiling.h"
#include "ports.h"

std::optional<User> IUserNetworkManager::getUserById(long long other_user_id) {
  const std::string path = "/users/" + std::to_string(other_user_id);
  auto res = forward(provider_->ports().userService, "", path, "GET");

  if (res.first != provider_->statusCodes().success) {
    LOG_ERROR("getUserById failed: {}", res.first);
    return std::nullopt;
  }

  try {
    const nlohmann::json obj = nlohmann::json::parse(res.second);

    if (!obj.is_object()) {
      LOG_ERROR("Invalid JSON format in getUserById");
      return std::nullopt;
    }

    if(!obj.contains("id")) {
      LOG_ERROR("Obj doesn't contain 'id' field");
      return std::nullopt;
    }

    if(!obj.contains("name")) {
      LOG_ERROR("Obj doesn't contain 'name' field");
      return std::nullopt;
    }

    if(!obj.contains("email")) {
      LOG_ERROR("Obj doesn't contain 'email' field");
      return std::nullopt;
    }

    if(!obj.contains("tag")) {
      LOG_ERROR("Obj doesn't contain 'tag' field");
      return std::nullopt;
    }

    User found_user{
        .id       = obj["id"],
        .username = obj["name"],
        .email    = obj["email"],
        .tag      = obj["tag"]
    };

    LOG_INFO("getUserById success: {}", nlohmann::json(found_user).dump());
    return found_user;
  }
  catch (const std::exception& e) {
    LOG_ERROR("JSON parse error in getUserById: {}", e.what());
    return std::nullopt;
  }
}
