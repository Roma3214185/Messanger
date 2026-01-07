#ifndef BACKEND_CHATSERVICE_SRC_HEADERS_TOKENSERVICE_H_
#define BACKEND_CHATSERVICE_SRC_HEADERS_TOKENSERVICE_H_

#include <optional>
#include <string>

namespace JwtUtils {

extern const char *kIssuer;
extern const std::string kKeysDir;
extern const std::string kPublicKeyFile;

std::optional<long long> verifyTokenAndGetUserId(const std::string &token);

} // namespace JwtUtils

#endif
