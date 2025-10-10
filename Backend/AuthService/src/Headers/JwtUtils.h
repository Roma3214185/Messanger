#ifndef JWTUTILS_H
#define JWTUTILS_H

#include "jwt-cpp/jwt.h"
#include <string>

namespace JwtUtils {

    std::string generateToken(int userId) {
        auto token = jwt::create()
        .set_issuer("my-amazon-clone")
        .set_type ("JWS")
        .set_payload_claim("sub", jwt::claim(std::to_string(userId)))
        .set_expires_at(std::chrono::system_clock::now() + std::chrono::minutes(30))
        .sign(jwt::algorithm::hs256{"super_secret_key"});
        return token;
    }

    std::optional<int> verifyTokenAndGetUserId(const std::string& token) {
        try {
            auto decoded = jwt::decode (token);
            auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{"super_secret_key"})
            .with_issuer("my-amazon-clone");
            verifier.verify(decoded) ;

            int userId = std::stoi(decoded.get_payload_claim("sub").as_string());
            return userId;
        }catch (const std:: exception& e) {
            return std::nullopt;
        }
    }

} // namespace JwtUtils

#endif // JWTUTILS_H
