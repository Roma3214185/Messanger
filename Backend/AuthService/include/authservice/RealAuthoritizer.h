#ifndef REALAUTHORITIZER_H
#define REALAUTHORITIZER_H

#include "interfaces/IAutoritizer.h"
#include "JwtUtils.h"

class RealAuthoritizer : public IAutoritizer {
  public:
    std::optional<long long> autoritize(const std::string& token) override {
      return JwtUtils::verifyTokenAndGetUserId(token);
    }
};


#endif // REALAUTHORITIZER_H
