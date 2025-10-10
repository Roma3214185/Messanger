#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <string>
#include "httplib.h"
#include <QVector>
#include <QJsonDocument>

namespace NetworkManager {

std::pair<int, std::string> forward(
    const std::string& body,
    const std::string& path,
    const std::string& method,
    const std::vector<std::pair<std::string, std::string>>& extra_headers = {})
{
    httplib::Headers headers;

    // Add any extra headers
    for (const auto& h : extra_headers) {
        headers.emplace(h.first, h.second);
    }

    const int CHAT_SERVICE = 8081;
    httplib::Client cli("localhost", CHAT_SERVICE);
    cli.set_connection_timeout(5, 0);  // 5 seconds timeout

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


QVector<int> getMembersOfChat(int chatId){

    QVector<int> members;
    std::string path = "/chats/" + std::to_string(chatId) + "/members";

    auto res = forward("", path, "GET");
    if (res.first != 200) {
        qDebug() << "[ERROR] getMembersOfChat failed:" << res.first;
        return members;
    }

    // Парсимо JSON-відповідь
    QByteArray responseData = QByteArray::fromStdString(res.second);
    QJsonDocument jsonResponse = QJsonDocument::fromJson(responseData);

    if (!jsonResponse.isObject()) {
        qDebug() << "[ERROR] Invalid JSON format in getMembersOfChat";
        return members;
    }

    QJsonObject obj = jsonResponse.object();
    if (!obj.contains("members") || !obj["members"].isArray()) {
        qDebug() << "[WARN] getMembersOfChat: no 'members' field found";
        return members;
    }

    QJsonArray arr = obj["members"].toArray();
    for (const auto& v : arr) {
        members.append(v.toInt());
    }

    qDebug() << "[INFO] getMembersOfChat success:" << members;
    return members;
}

} // namespace NetworkManager

#endif // NETWORKMANAGER_H
