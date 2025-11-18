#include "interfaces/IChatNetworkManager.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "Debug_profiling.h"

QVector<UserId> IChatNetworkManager::getMembersOfChat(int chat_id) {
  QVector<UserId> members;
  std::string     path = "/chats/" + std::to_string(chat_id) + "/members";

  auto res = forward(provider_->ports().chatService, "", path, "GET");
  if (res.first != provider_->statusCodes().success) {
    LOG_ERROR("GetMembersOfChat failed '{}' and reason: '{}' ", res.first, res.second);
    return members;
  }

  auto response_data = QByteArray::fromStdString(res.second);
  auto json_response = QJsonDocument::fromJson(response_data);

  if (!json_response.isObject()) {
    LOG_ERROR("Invalid JSON format in getMembersOfChat");
    return members;
  }

  auto obj = json_response.object();
  if (!obj.contains("members") || !obj["members"].isArray()) {
    LOG_ERROR("getMembersOfChat: no 'members' field found");
    return members;
  }

  auto arr = obj["members"].toArray();
  for (const auto& v : arr) {
    members.append(v.toInt());
  }

  LOG_INFO("getMembersOfChat success '{}'", members.size());
  return members;
}
