#include "JWTVerifier.h"

#include <fstream>

namespace {

std::string readFile(const std::string& path) {
  LOG_INFO("Loading JWT public key from {}", path);

  const std::ifstream file(path, std::ios::binary);  // read as binary
  if (!file.is_open()) throw std::runtime_error("Cannot open file: " + path);

  std::stringstream sstream;
  sstream << file.rdbuf();
  std::string content = sstream.str();

  LOG_INFO("Read {} bytes", content.size());
  if (content.empty()) throw std::runtime_error("File is empty: " + path);

  LOG_INFO("Key starts with: {}", content.substr(0, 30));
  return content;
}

}  // namespace

JWTVerifier::JWTVerifier(const std::string& public_key_path, const std::string& issuer)
    : public_key_(readFile(public_key_path)),  // todo: can be excaption in readFile
      verifier_(jwt::verify()
                    .allow_algorithm(jwt::algorithm::rs256(public_key_, "", "", ""))
                    .with_issuer(issuer)) {
  LOG_INFO("JWTVerifier initialized successfully.");
}

std::optional<long long> JWTVerifier::verifyTokenAndGetUserId(const std::string& token) {
  try {
    auto decoded = jwt::decode(token);
    verifier_.verify(decoded);

    long long user_id = std::stoll(decoded.get_payload_claim("sub").as_string());  // todo: try
                                                                                   // catch
    return user_id;
  } catch (const std::exception& e) {
    LOG_ERROR("Token verification failed: {}", e.what());
    return std::nullopt;
  }
}
