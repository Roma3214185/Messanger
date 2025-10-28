#ifndef AUTHVERIFIER_H
#define AUTHVERIFIER_H

#include "ProxyClient/proxyclient.h"

using IsValid = bool;
using UserJsonString = std::string;
using VerificationResult = std::pair<bool, std::optional<UserJsonString>>;

struct AuthVerifier {
  ProxyClient client;

  explicit AuthVerifier(const std::string& url) : client(url) {}

  VerificationResult verify(const std::string& token) {
    nlohmann::json body = {{"token", token}};
    auto [status, response] = client.post_json("/verify", body);
    constexpr int kSuccessfulCode = 200;
    const VerificationResult kError = {false, std::nullopt};

    if (status != kSuccessfulCode) return kError;

    try {
        auto j = nlohmann::json::parse(response);

      if (j.value("valid", false)) {
        if (j.contains("user")) {
          return {true, j["user"].dump()};
        } else {
          return kError;
        }
      }

    } catch (const std::exception& e) {
      std::cout << e.what() << std::endl;
      return kError;
    }

    return kError;
  }
};

#endif  // AUTHVERIFIER_H
