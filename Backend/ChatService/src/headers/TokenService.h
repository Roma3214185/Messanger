#ifndef TOKENSERVICE_H
#define TOKENSERVICE_H

#include <optional>
#include "jwt-cpp/jwt.h"

namespace TokenService {

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

} // namespace TokenService

#endif // TOKENSERVICE_H
