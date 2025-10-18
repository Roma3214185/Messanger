#ifndef JWTUTILS_H
#define JWTUTILS_H

#include "jwt-cpp/jwt.h"
#include <string>
#include "../../../DebugProfiling/Debug_profiling.h"

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
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(24*365*10))
        .sign(jwt::algorithm::hs256{SECRET_KEY});
}

OptionalUserId verifyTokenAndGetUserId(const std::string& token) {
    try {
        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
                            .allow_algorithm(jwt::algorithm::hs256{SECRET_KEY})
                            .with_issuer(ISSUER);
        verifier.verify(decoded);

        int userId = std::stoll(decoded.get_payload_claim("sub").as_string());
        LOG_INFO("Token is verified, id is '{}'", userId);
        return userId;
    } catch (const std::exception& e) {
        LOG_ERROR("Error verifying token, error: {}", e.what());
        return std::nullopt;
    } catch (...) {
        LOG_ERROR("Unknown error verifying token");
        return std::nullopt;
    }
}

} // namespace JwtUtils

#endif // JWTUTILS_H
