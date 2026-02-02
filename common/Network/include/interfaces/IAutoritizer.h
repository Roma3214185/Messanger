#ifndef IAUTHORITIZER_H
#define IAUTHORITIZER_H

#include <optional>

class IAuthoritizer {
 public:
  virtual ~IAuthoritizer() = default;
  virtual std::optional<long long> verifyTokenAndGetUserId(const std::string &token) = 0;
};

#endif  // IAUTHORITIZER_H
