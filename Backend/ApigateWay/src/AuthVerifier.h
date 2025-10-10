#ifndef AUTHVERIFIER_H
#define AUTHVERIFIER_H

#include "proxyclient.h"

using json = nlohmann::json;

struct AuthVerifier {
    ProxyClient client;
    AuthVerifier(const std::string& url): client(url) {}

    // returns pair(valid, user_json_string)
    std::pair<bool, std::string> verify(const std::string& token) {
        json body = { {"token", token} };
        auto r = client.post_json("/verify", body);
        if (r.first != 200) return {false, ""};

        try {
            auto j = json::parse(r.second);
            if (j.contains("valid") && j["valid"].get<bool>()) {
                if (j.contains("user")) return {true, j["user"].dump()};
                return {true, ""};
            }
        } catch (...) {
            return {false, ""};
        }
        return {false, ""};
    }
};


#endif // AUTHVERIFIER_H
