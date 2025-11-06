#include "JwtUtils.h"

#include <jwt-cpp/jwt.h>
#include <fstream>

#include "Debug_profiling.h"

const std::string kKeysDir = "/Users/roma/QtProjects/Chat/Backend/shared_keys/";
const std::string kPublicKeyFile = kKeysDir + "public_key.pem";
inline constexpr const char* kIssuer = "auth_service";

namespace {

std::string readFile(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) throw std::runtime_error("Cannot open file " + path);
  return std::string((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
}

}  // namespace

namespace JwtUtils {

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

}  // namespace TokenService
