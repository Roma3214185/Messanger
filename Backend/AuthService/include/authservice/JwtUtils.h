#ifndef BACKEND_AUTHSERVICE_SRC_HEADERS_JWTUTILS_H_
#define BACKEND_AUTHSERVICE_SRC_HEADERS_JWTUTILS_H_

#include <jwt-cpp/jwt.h>

#include <chrono>
#include <optional>
#include <string>

#include "Debug_profiling.h"

using UserId         = long long;
using OptionalUserId = std::optional<UserId>;

namespace JwtUtils {

std::pair<std::string, std::string> generate_rsa_keys(int bits = 2048);
std::string                         generateToken(UserId);
OptionalUserId                      verifyTokenAndGetUserId(const std::string& token);

}  // namespace JwtUtils

#endif  // BACKEND_AUTHSERVICE_SRC_HEADERS_JWTUTILS_H_
