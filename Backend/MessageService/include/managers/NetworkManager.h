#ifndef BACKEND_MESSAGESERVICE_HEADERS_NETWORKMANAGER_H_
#define BACKEND_MESSAGESERVICE_HEADERS_NETWORKMANAGER_H_

#include <QVector>
#include <string>
#include <utility>
#include <vector>

using UserId = int;

namespace NetworkManager {

std::pair<int, std::string> forward(
    const std::string&                                      body,
    const std::string&                                      path,
    const std::string&                                      method,
    const std::vector<std::pair<std::string, std::string>>& extra_headers = {});

QVector<UserId> getMembersOfChat(int chat_id);

}  // namespace NetworkManager

#endif  // BACKEND_MESSAGESERVICE_HEADERS_NETWORKMANAGER_H_
