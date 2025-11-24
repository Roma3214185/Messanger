#include "JWTVerifier.h"

#include <fstream>

namespace {

static std::string readFile(const std::string& path) {
  LOG_INFO("Loading JWT public key from {}", path);

  std::ifstream file(path, std::ios::binary); // read as binary
  if (!file.is_open())
    throw std::runtime_error("Cannot open file: " + path);

  std::stringstream ss;
  ss << file.rdbuf();
  std::string content = ss.str();

  LOG_INFO("Read {} bytes", content.size());
  if (content.size() == 0)
    throw std::runtime_error("File is empty: " + path);

  LOG_INFO("Key starts with: {}", content.substr(0, 30));
  return content;
}


}  // namespace

JWTVerifier::JWTVerifier(const std::string& publicKeyPath, const std::string& issuer)
    : publicKey_(readFile(publicKeyPath)),
    verifier_(jwt::verify()
                  .allow_algorithm(jwt::algorithm::rs256(publicKey_, "", "", ""))
                  .with_issuer(issuer))
{
  LOG_INFO("JWTVerifier initialized successfully.");
}

std::optional<int> JWTVerifier::verifyTokenAndGetUserId(const std::string& token)
{
  try {
    auto decoded = jwt::decode(token);
    verifier_.verify(decoded);

    int user_id = std::stoi(decoded.get_payload_claim("sub").as_string());
    return user_id;
  }
  catch (const std::exception& e) {
    LOG_ERROR("Token verification failed: {}", e.what());
    return std::nullopt;
  }
}
