#include "interfaces/IUserNetworkManager.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "entities/User.h"
#include "Debug_profiling.h"
#include "ports.h"

std::optional<User> IUserNetworkManager::getUserById(int otherUserId) {
  std::string path = "/users/" + std::to_string(otherUserId);
  auto        res  = forward(provider_->ports().userService, "", path, "GET");

  if (res.first != provider_->statusCodes().success) {
    LOG_ERROR("getUserById failed: {}", res.first);
    return std::nullopt;
  }

  auto responseData = QByteArray::fromStdString(res.second);
  auto jsonResponse = QJsonDocument::fromJson(responseData);

  if (!jsonResponse.isObject()) {
    LOG_ERROR("Invalid JSON format in getUserById");
    return std::nullopt;
  }

  auto obj = jsonResponse.object();

  User findedUser{.id       = obj["id"].toInt(),
                  .email    = obj["email"].toString().toStdString(),
                  .username = obj["name"].toString().toStdString(),
                  .tag      = obj["tag"].toString().toStdString()};

  LOG_INFO("getUserById success: {}", findedUser.username);
  return findedUser;
}
