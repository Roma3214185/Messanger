#include "managers/NetworkManager.h"

#include <httplib.h>
#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVector>

#include <memory>
#include <string>
#include <utility>
#include <vector>

using std::pair;
using std::string;
using std::unique_ptr;
using std::vector;

constexpr int kFailStatusCode = 502;
constexpr int kSuccessStatusCode = 200;

namespace NetworkManager {

pair<int, string> forward(const string& body, const string& path,
                          const string& method,
                          const vector<pair<string, string>>& extra_headers) {
  httplib::Headers headers;
  for (const auto& h : extra_headers) {
    headers.emplace(h.first, h.second);
  }

  constexpr int kChatService = 8081;
  httplib::Client cli("localhost", kChatService);
  cli.set_connection_timeout(5, 0);

  httplib::Result res(unique_ptr<httplib::Response>(nullptr),
                      httplib::Error::Unknown);

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
    return {kFailStatusCode, "Bad Gateway: downstream no response"};
  }

  return {res->status, res->body};
}

QVector<UserId> getMembersOfChat(int chat_id) {
  QVector<UserId> members;
  string path = "/chats/" + std::to_string(chat_id) + "/members";

  auto res = forward("", path, "GET");
  if (res.first != kSuccessStatusCode) {
    qDebug() << "[ERROR] getMembersOfChat failed:" << res.first;
    return members;
  }

  auto responseData = QByteArray::fromStdString(res.second);
  auto jsonResponse = QJsonDocument::fromJson(responseData);

  if (!jsonResponse.isObject()) {
    qDebug() << "[ERROR] Invalid JSON format in getMembersOfChat";
    return members;
  }

  auto obj = jsonResponse.object();
  if (!obj.contains("members") || !obj["members"].isArray()) {
    qDebug() << "[WARN] getMembersOfChat: no 'members' field found";
    return members;
  }

  auto arr = obj["members"].toArray();
  for (const auto& v : std::as_const(arr)) {
    members.append(v.toInt());
  }

  qDebug() << "[INFO] getMembersOfChat success:" << members;
  return members;
}

}  // namespace NetworkManager
