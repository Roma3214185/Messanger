#include "interfaces/IUserNetworkManager.h"

#include "Debug_profiling.h"
#include "config/codes.h"
#include "config/ports.h"
#include "entities/User.h"
#include "proxyclient.h"
#include "ForwardRequestDTO.h"
#include "entities/RequestDTO.h"

UserNetworkManager::UserNetworkManager(ProxyClient* proxy) : proxy_(proxy) {}

std::optional<User> UserNetworkManager::getUserById(long long other_user_id) {
  const std::string path = "/users/" + std::to_string(other_user_id);
  RequestDTO request;
  request.method = "GET";
  request.path = path;
  auto [code, body] = proxy_->forward(request, Config::Ports::userService);

  if (code != Config::StatusCodes::success) {
    LOG_ERROR("getUserById failed: {}", code);
    return std::nullopt;
  }

  try {
    const nlohmann::json obj = nlohmann::json::parse(body);

    if (!obj.is_object()) {
      LOG_ERROR("Invalid JSON format in getUserById");
      return std::nullopt;
    }

    if (!obj.contains("id")) {
      LOG_ERROR("Obj doesn't contain 'id' field");
      return std::nullopt;
    }

    if (!obj.contains("name")) {
      LOG_ERROR("Obj doesn't contain 'name' field");
      return std::nullopt;
    }

    if (!obj.contains("email")) {
      LOG_ERROR("Obj doesn't contain 'email' field");
      return std::nullopt;
    }

    if (!obj.contains("tag")) {
      LOG_ERROR("Obj doesn't contain 'tag' field");
      return std::nullopt;
    }

    User found_user(obj["id"], obj["name"], obj["email"], obj["tag"]);

    LOG_INFO("getUserById success: {}", nlohmann::json(found_user).dump());
    return found_user;
  } catch (const std::exception &e) {
    LOG_ERROR("JSON parse error in getUserById: {}", e.what());
    return std::nullopt;
  }
}
