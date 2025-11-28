#include "interfaces/IUserNetworkManager.h"
#include <nlohmann/json.hpp>

#include "entities/User.h"
#include "Debug_profiling.h"
#include "ports.h"

using json = nlohmann::json;

std::optional<User> IUserNetworkManager::getUserById(int otherUserId) {
  std::string path = "/users/" + std::to_string(otherUserId);
  auto res = forward(provider_->ports().userService, "", path, "GET");

  if (res.first != provider_->statusCodes().success) {
    LOG_ERROR("getUserById failed: {}", res.first);
    return std::nullopt;
  }

  try {
    json obj = json::parse(res.second);

    if (!obj.is_object()) {
      LOG_ERROR("Invalid JSON format in getUserById");
      return std::nullopt;
    }

    User foundUser{
        .id       = obj.value("id", -1),
        .username = obj.value("name", ""),
        .email    = obj.value("email", ""),
        .tag      = obj.value("tag", "")
    };

    LOG_INFO("getUserById success: {}", foundUser.username);
    return foundUser;
  }
  catch (const std::exception& e) {
    LOG_ERROR("JSON parse error in getUserById: {}", e.what());
    return std::nullopt;
  }
}
