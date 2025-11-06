#ifndef APIGATE_SERVICE_HEADERS_JWTUTILS_H
#define APIGATE_SERVICE_HEADERS_JWTUTILS_H

#include <jwt-cpp/jwt.h>
#include <optional>
#include "Debug_profiling.h"


namespace JwtUtils {

std::optional<int> verifyTokenAndGetUserId(const std::string& token);

}  // namespace TokenService

#endif // APIGATE_SERVICE_HEADERS_JWTUTILS_H

