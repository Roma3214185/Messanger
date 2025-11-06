#ifndef MESSAGE_SERVICE_HEADERS_JWTUTILS_H
#define MESSAGE_SERVICE_HEADERS_JWTUTILS_H

#include <optional>
#include <string>

namespace JwtUtils {

std::optional<int> verifyTokenAndGetUserId(const std::string& token);

}  // namespace TokenService

#endif // MESSAGE_SERVICE_HEADERS_JWTUTILS_H
