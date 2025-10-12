#ifndef AUTHVERIFIER_H
#define AUTHVERIFIER_H

#include "ProxyClient/proxyclient.h"

using json = nlohmann::json;
using IsValid =  bool;
using UserJsonString = std::string;
using VerificationResult = std::pair<bool, std::optional<UserJsonString>>;

struct AuthVerifier
{
    ProxyClient client;

    explicit AuthVerifier(const std::string& url): client(url) {}

    VerificationResult verify(const std::string& token) {
        json body = { {"token", token} };
        auto [status, response] = client.post_json("/verify", body);

        constexpr int successfulCode = 200;
        if (status != successfulCode) return {false, std::nullopt};

        try {
            auto j = json::parse(response);

            if (j.value("valid", false)) {
                if (j.contains("user")) {
                    return {true, j["user"].dump()};
                } else {
                    return {true, std::nullopt};
                }
            }

        } catch (const std::exception& e) {
            std::cout << e.what() << std::endl;
            return {false, std::nullopt};
        }

        return {false, std::nullopt};
    }
};


#endif // AUTHVERIFIER_H
