#ifndef BACKEND_AUTHSERVICE_SRC_HEADERS_JWTUTILS_H_
#define BACKEND_AUTHSERVICE_SRC_HEADERS_JWTUTILS_H_

#include <jwt-cpp/jwt.h>

#include <chrono>
#include <string>
#include <optional>

#include "Debug_profiling.h"

using UserId = int;
using OptionalUserId = std::optional<UserId>;

namespace JwtUtils {

inline constexpr const char* kSecretKey = "super_secret_key";
inline constexpr const char* kIssuer = "my-amazon-clone";
constexpr int kTenYears = 34 * 365 * 10;

std::string generateToken(int userId) {
  return jwt::create()
      .set_issuer(kIssuer)
      .set_type("JWS")
      .set_payload_claim("sub", jwt::claim(std::to_string(userId)))
      .set_expires_at(std::chrono::system_clock::now() +
                      std::chrono::hours(kTenYears))
      .sign(jwt::algorithm::hs256{kSecretKey});
}

OptionalUserId verifyTokenAndGetUserId(const std::string& token) {
  try {
    auto decoded = jwt::decode(token);
    auto verifier = jwt::verify()
                        .allow_algorithm(jwt::algorithm::hs256{kSecretKey})
                        .with_issuer(kIssuer);
    verifier.verify(decoded);

    int user_id = std::stoll(decoded.get_payload_claim("sub").as_string());
    LOG_INFO("Token is verified, id is '{}'", user_id);
    return user_id;
  } catch (const std::exception& e) {
    LOG_ERROR("Error verifying token, error: {}", e.what());
    return std::nullopt;
  } catch (...) {
    LOG_ERROR("Unknown error verifying token");
    return std::nullopt;
  }
}

}  // namespace JwtUtils

#endif  // BACKEND_AUTHSERVICE_SRC_HEADERS_JWTUTILS_H_
