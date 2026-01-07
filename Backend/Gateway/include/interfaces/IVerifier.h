#ifndef IVERIFIER_H
#define IVERIFIER_H

#include <optional>
#include <string>

class IVerifier {
public:
  virtual std::optional<long long>
  verifyTokenAndGetUserId(const std::string &token) = 0;
  virtual ~IVerifier() = default;
};

#endif // IVERIFIER_H
