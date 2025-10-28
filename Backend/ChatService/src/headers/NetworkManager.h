#ifndef BACKEND_CHATSERVICE_SRC_HEADERS_NETWORKMANAGER_H_
#define BACKEND_CHATSERVICE_SRC_HEADERS_NETWORKMANAGER_H_

#include <optional>
#include <string>
#include <vector>

#include "user.h"

namespace NetworkManager {

std::pair<int, std::string> forward(
    const std::string& body, const std::string& path, const std::string& method,
    const std::vector<std::pair<std::string, std::string>>& extra_headers = {});

std::optional<User> getUserById(int otherUserId);

}  // namespace NetworkManager

#endif  // BACKEND_CHATSERVICE_SRC_HEADERS_NETWORKMANAGER_H_
