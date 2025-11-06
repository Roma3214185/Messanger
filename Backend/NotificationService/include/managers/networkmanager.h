#ifndef BACKEND_NOTIFICATIONSERVICE_NETWORKMANAGER_NETWORKMANAGER_H_
#define BACKEND_NOTIFICATIONSERVICE_NETWORKMANAGER_NETWORKMANAGER_H_

#include <httplib.h>

#include <QVector>
#include <string>

#include "Debug_profiling.h"

using UserId = int;
using StatusCode = int;
using ResponceBody = std::string;
using Responce = std::pair<StatusCode, ResponceBody>;

class NetworkManager {
 public:
  Responce forward(const std::string& body, const std::string& path,
                   const std::string& method,
                   const std::vector<std::pair<std::string, std::string>>&
                       extra_headers = {});
  QVector<UserId> getMembersOfChat(int chat_id);
};

#endif  // BACKEND_NOTIFICATIONSERVICE_NETWORKMANAGER_NETWORKMANAGER_H_
