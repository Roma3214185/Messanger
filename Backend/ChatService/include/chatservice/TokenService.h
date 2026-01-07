#ifndef BACKEND_CHATSERVICE_SRC_HEADERS_TOKENSERVICE_H_
#define BACKEND_CHATSERVICE_SRC_HEADERS_TOKENSERVICE_H_

#include <optional>
#include <string>

namespace JwtUtils {

std::optional<long long> verifyTokenAndGetUserId(const std::string &token);

} // namespace JwtUtils

#endif
