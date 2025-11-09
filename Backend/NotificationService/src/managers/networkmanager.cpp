#include "managers/networkmanager.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

Responce NetworkManager::forward(
    const std::string&                                      body,
    const std::string&                                      path,
    const std::string&                                      method,
    const std::vector<std::pair<std::string, std::string>>& extra_headers) {
  httplib::Headers headers;
  for (const auto& h : extra_headers) {
    headers.emplace(h.first, h.second);
  }

  const int       CHAT_SERVICE = 8081;
  httplib::Client cli("localhost", CHAT_SERVICE);
  cli.set_connection_timeout(5, 0);

  httplib::Result res(std::unique_ptr<httplib::Response>(nullptr), httplib::Error::Unknown);

  if (method == "GET") {
    res = cli.Get(path.c_str(), headers);
  } else if (method == "DELETE") {
    res = cli.Delete(path.c_str(), headers);
  } else if (method == "PUT") {
    res = cli.Put(path.c_str(), headers, body, "application/json");
  } else {
    res = cli.Post(path.c_str(), headers, body, "application/json");
  }

  if (!res) {
    return {502, "Bad Gateway: downstream no response"};
  }

  return {res->status, res->body};
}

QVector<UserId> NetworkManager::getMembersOfChat(int chat_id) {
  QVector<UserId> members;
  std::string     path = "/chats/" + std::to_string(chat_id) + "/members";

  auto res = forward("", path, "GET");
  if (res.first != 200) {
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
