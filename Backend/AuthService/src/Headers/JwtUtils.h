#ifndef JWTUTILS_H
#define JWTUTILS_H

#include "jwt-cpp/jwt.h"
#include <string>

using UserId = int;
using OptionalUserId = std::optional<UserId>;

namespace JwtUtils {

inline constexpr const char* SECRET_KEY = "super_secret_key";
inline constexpr const char* ISSUER = "my-amazon-clone";

std::string generateToken(int userId) {
    return jwt::create()
        .set_issuer(ISSUER)
        .set_type("JWS")
        .set_payload_claim("sub", jwt::claim(std::to_string(userId)))
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::minutes(30))
        .sign(jwt::algorithm::hs256{SECRET_KEY});
}

OptionalUserId verifyTokenAndGetUserId(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
                            .allow_algorithm(jwt::algorithm::hs256{SECRET_KEY})
                            .with_issuer(ISSUER);
        verifier.verify(decoded) ;

        int userId = std::stoi(decoded.get_payload_claim("sub").as_string());
        return userId;
    } catch (...) {
        return std::nullopt;
    }
}

} // namespace JwtUtils

#endif // JWTUTILS_H
