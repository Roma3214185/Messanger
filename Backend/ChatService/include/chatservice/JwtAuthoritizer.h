#ifndef BACKEND_CHATSERVICE_JWTAUTHORITIZER_H_
#define BACKEND_CHATSERVICE_JWTAUTHORITIZER_H_

#include <optional>
#include <string>

#include "interfaces/IAutoritizer.h"

class JwtAuthoritizer : public IAuthoritizer {
 public:
    std::optional<long long> verifyTokenAndGetUserId(const std::string &token) override;
};

#endif  // BACKEND_CHATSERVICE_JWTAUTHORITIZER_H_
