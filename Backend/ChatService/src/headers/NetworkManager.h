#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <string>
#include <vector>
#include <optional>

#include "user.h"

namespace NetworkManager {

std::pair<int, std::string> forward(
    const std::string& body,
    const std::string& path,
    const std::string& method,
    const std::vector<std::pair<std::string, std::string>>& extra_headers = {});

std::optional<User> getUserById(int otherUserId);

} // namespace NetworkManager

#endif // NETWORKMANAGER_H
