#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <string>
#include "httplib.h"
#include <QVector>
#include <QJsonDocument>

using UserId = int;

namespace NetworkManager {

std::pair<int, std::string> forward(
    const std::string& body,
    const std::string& path,
    const std::string& method,
    const std::vector<std::pair<std::string, std::string>>& extra_headers = {})
{
    httplib::Headers headers;
    for (const auto& h : extra_headers) {
        headers.emplace(h.first, h.second);
    }

    const int CHAT_SERVICE = 8081;
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

    return { res->status, res->body };
}


QVector<UserId> getMembersOfChat(int chatId){
    QVector<UserId> members;
    std::string path = "/chats/" + std::to_string(chatId) + "/members";

    auto res = forward("", path, "GET");
    if (res.first != 200) {
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
    for (const auto& v : arr) {
        members.append(v.toInt());
    }

    qDebug() << "[INFO] getMembersOfChat success:" << members;
    return members;
}

} // namespace NetworkManager

#endif // NETWORKMANAGER_H
