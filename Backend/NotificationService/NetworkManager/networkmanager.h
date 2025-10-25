#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <string>
#include <httplib.h>
#include <QVector>
#include "Debug_profiling.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

using UserId = int;
using StatusCode = int;
using ResponceBody = std::string;
using Responce = std::pair<StatusCode, ResponceBody>;

class NetworkManager
{
public:

    Responce forward(const std::string& body,
                    const std::string& path,
                    const std::string& method,
                    const std::vector<std::pair<std::string, std::string>>& extra_headers = {});

    QVector<UserId> getMembersOfChat(int chatId);
};

#endif // NETWORKMANAGER_H
