#ifndef AUTHVERIFIER_H
#define AUTHVERIFIER_H

#include "ProxyClient/proxyclient.h"

#include <jwt-cpp/jwt.h>

#include <optional>

using IsValid = bool;
using UserJsonString = std::string;
using VerificationResult = std::pair<bool, std::optional<UserJsonString>>;

struct AuthVerifier {
  ProxyClient client;

  explicit AuthVerifier(const std::string& url) : client(url) {}

  std::optional<int> verifyTokenAndGetUserId(const std::string& token) {
    try {
      auto decoded = jwt::decode(token);
      auto verifier =
          jwt::verify()
              .allow_algorithm(jwt::algorithm::hs256{"super_secret_key"})
              .with_issuer("my-amazon-clone");
      verifier.verify(decoded);

      int userId = std::stoi(decoded.get_payload_claim("sub").as_string());
      return userId;
    } catch (const std::exception& e) {
      return std::nullopt;
    }
  }

  // VerificationResult verify(const std::string& token) {
  //   nlohmann::json body = {{"token", token}};
  //   auto [status, response] = client.post_json("/verify", body);
  //   constexpr int kSuccessfulCode = 200;
  //   const VerificationResult kError = {false, std::nullopt};

  //   if (status != kSuccessfulCode) return kError;

  //   try {
  //       auto j = nlohmann::json::parse(response);

  //     if (j.value("valid", false)) {
  //       if (j.contains("user")) {
  //         return {true, j["user"].dump()};
  //       } else {
  //         return kError;
  //       }
  //     }

  //   } catch (const std::exception& e) {
  //     std::cout << e.what() << std::endl;
  //     return kError;
  //   }

  //   return kError;
  // }
};

#endif  // AUTHVERIFIER_H
