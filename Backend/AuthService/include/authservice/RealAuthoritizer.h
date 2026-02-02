#ifndef REALAUTHORITIZER_H
#define REALAUTHORITIZER_H

#include "JwtUtils.h"
#include "interfaces/IAutoritizer.h"

class RealAuthoritizer : public IAuthoritizer {
 public:
    std::optional<long long> verifyTokenAndGetUserId(const std::string &token) override;
};

#endif  // REALAUTHORITIZER_H
