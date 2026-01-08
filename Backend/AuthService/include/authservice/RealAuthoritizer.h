#ifndef REALAUTHORITIZER_H
#define REALAUTHORITIZER_H

#include "JwtUtils.h"
#include "interfaces/IAutoritizer.h"

class RealAuthoritizer : public IAutoritizer {
 public:
  std::optional<long long> autoritize(const std::string &token) override {
    return JwtUtils::verifyTokenAndGetUserId(token);
  }
};

#endif  // REALAUTHORITIZER_H
