#ifndef BACKEND_CHATSERVICE_SRC_HEADERS_TOKENSERVICE_H_
#define BACKEND_CHATSERVICE_SRC_HEADERS_TOKENSERVICE_H_

#include <jwt-cpp/jwt.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <memory>
#include "Debug_profiling.h"

namespace {

std::string readFile(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) throw std::runtime_error("Cannot open file " + path);
  auto key =  std::string((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
  LOG_INFO("Key first 40 chars:\n{}", key.substr(0,40));
  LOG_INFO("Key last 40 chars:\n{}", key.substr(key.size()-40,40));
  return key;
}

}  // namespace

namespace JwtUtils {

inline constexpr const char* kIssuer = "auth_service";
const std::string kKeysDir = "/Users/roma/QtProjects/Chat/Backend/shared_keys/";
const std::string kPublicKeyFile = kKeysDir + "public_key.pem";

std::optional<int> verifyTokenAndGetUserId(const std::string& token) {
  try {
    auto decoded = jwt::decode(token);
    std::string public_key = readFile(kPublicKeyFile);

    auto verifier = jwt::verify()
                        .allow_algorithm(jwt::algorithm::rs256(public_key, "", "", ""))
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

#endif  // BACKEND_CHATSERVICE_SRC_HEADERS_TOKENSERVICE_H_







