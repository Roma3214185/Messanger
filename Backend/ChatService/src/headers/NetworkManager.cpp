#include "NetworkManager.h"
#include <httplib.h>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

const int APIGATEWAY_PORT = 8084;
const int AUTHSERVICE_PORT = 8083;

namespace NetworkManager {

std::pair<int, std::string> forward(
    const std::string& body,
    const std::string& path,
    const std::string& method,
    const std::vector<std::pair<std::string, std::string>>& extra_headers)
{
    httplib::Headers headers;
    for (const auto& h : extra_headers) {
        headers.emplace(h.first, h.second);
    }

    httplib::Client cli("localhost", AUTHSERVICE_PORT);
    cli.set_connection_timeout(5, 0);

    httplib::Result res(std::unique_ptr<httplib::Response>(nullptr), httplib::Error::Unknown);

    if (method == "GET") res = cli.Get(path.c_str(), headers);
    else if (method == "DELETE") res = cli.Delete(path.c_str(), headers);
    else if (method == "PUT") res = cli.Put(path.c_str(), headers, body, "application/json");
    else res = cli.Post(path.c_str(), headers, body, "application/json");

    if (!res) {
        return {502, "Bad Gateway: downstream no response"};
    }

    return { res->status, res->body };
}

std::optional<User> getUserById(int otherUserId) {
    std::string path = "/users/" + std::to_string(otherUserId);
    auto res = forward("", path, "GET");

    if (res.first != 200) {
        qDebug() << "[ERROR] getUserById failed:" << res.first;
        return std::nullopt;
    }

    auto responseData = QByteArray::fromStdString(res.second);
    auto jsonResponse = QJsonDocument::fromJson(responseData);

    if (!jsonResponse.isObject()) {
        qDebug() << "[ERROR] Invalid JSON format in getUserById";
        return std::nullopt;
    }

    auto obj = jsonResponse.object();

    User findedUser{
        .id = obj["id"].toInt(),
        .email = obj["email"].toString().toStdString(),
        .name =  obj["name"].toString().toStdString(),
        .tag = obj["tag"].toString().toStdString()
    };

    qDebug() << "[INFO] getUserById success:" << findedUser.name;
    return findedUser;
}

} // namespace NetworkManager
