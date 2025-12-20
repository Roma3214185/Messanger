#ifndef APIGATE_SERVICE_HEADERS_JWTUTILS_H
#define APIGATE_SERVICE_HEADERS_JWTUTILS_H

#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/kazuho-picojson/defaults.h>
#include <jwt-cpp/traits/kazuho-picojson/traits.h>

#include <optional>

#include "interfaces/IVerifier.h"
#include "Debug_profiling.h"

class JWTVerifier : public IVerifier {
  public:
    explicit JWTVerifier(const std::string& publicKeyPath, const std::string& issuer);

    std::optional<long long> verifyTokenAndGetUserId(const std::string& token) override;

  private:
    std::string publicKey_;
    jwt::verifier<jwt::default_clock, jwt::traits::kazuho_picojson> verifier_;
};

#endif  // APIGATE_SERVICE_HEADERS_JWTUTILS_H
