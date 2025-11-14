#include "interfaces/IMessageNetworkManager.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include "Debug_profiling.h"
#include "ports.h"
#include "codes.h"

QVector<UserId> IMessageNetworkManager::getMembersOfChat(int chat_id) {
  QVector<UserId> members;
  std::string path = "/chats/" + std::to_string(chat_id) + "/members";

  auto res = forward(ports::MessageServicePort, "", path, "GET");
  if (res.first != codes::success) {
    qDebug() << "[ERROR] getMembersOfChat failed:" << res.first;
    return members;
  }

  auto responseData = QByteArray::fromStdString(res.second);
  auto jsonResponse = QJsonDocument::fromJson(responseData);

  if (!jsonResponse.isObject()) {
    LOG_ERROR("Invalid JSON format in getMembersOfChat");
    return members;
  }

  auto obj = jsonResponse.object();
  if (!obj.contains("members") || !obj["members"].isArray()) {
    LOG_WARN("getMembersOfChat: no 'members' field found");
    return members;
  }

  auto arr = obj["members"].toArray();
  for (const auto& v : std::as_const(arr)) {
    members.append(v.toInt());
  }

  LOG_INFO("getMembersOfChat success: {}", members.size());
  return members;
}
