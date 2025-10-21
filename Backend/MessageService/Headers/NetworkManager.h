#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <string>
#include "httplib.h"
#include <QVector>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

using UserId = int;

namespace NetworkManager {

std::pair<int, std::string> forward(
    const std::string& body,
    const std::string& path,
    const std::string& method,
    const std::vector<std::pair<std::string, std::string>>& extra_headers = {});

QVector<UserId> getMembersOfChat(int chatId);

} // namespace NetworkManager

#endif // NETWORKMANAGER_H
