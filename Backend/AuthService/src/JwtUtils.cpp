#include "authservice/JwtUtils.h"

#include <jwt-cpp/jwt.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>

#include <fstream>

namespace {

std::string readFile(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) throw std::runtime_error("Cannot open file " + path);
  auto key = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
  LOG_INFO("Private key first 40 chars:\n{}", key.substr(0, 40));
  LOG_INFO("Private key last 40 chars:\n{}", key.substr(key.size() - 40, 40));
  return key;
}

}  // namespace

namespace JwtUtils {

inline constexpr const char* kIssuer         = "auth_service";
constexpr int                kTenYears       = 24 * 365 * 10;
const std::string            kPublicKeysDir  = "/Users/roma/QtProjects/Chat/Backend/shared_keys/";
const std::string            kPtivateKeysDir = "/Users/roma/QtProjects/Chat/Backend/AuthService/private_keys/";
const std::string            kPrivateKeyFile = kPtivateKeysDir + "private_key.pem";
const std::string            kPublicKeyFile  = kPublicKeysDir + "public_key.pem";

std::string generateToken(UserId user_id) {
  auto private_key = readFile(kPrivateKeyFile);
  try {
    auto token =
        jwt::create()
            .set_type("JWT")
            .set_payload_claim("sub", jwt::claim(std::to_string(user_id)))
            .set_issuer(kIssuer)
            .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours(kTenYears))
            .sign(jwt::algorithm::rs256("", private_key, ""));
    LOG_ERROR("JWT signing succeessed: {}", token);
    return token;
  } catch (const std::exception& e) {
    LOG_ERROR("JWT signing failed: {}", e.what());
    throw;
  }
}

std::optional<long long> verifyTokenAndGetUserId(const std::string& token) {
  try {
    auto        decoded    = jwt::decode(token);
    const std::string public_key = readFile(kPublicKeyFile);

    auto verifier = jwt::verify()
                        .allow_algorithm(jwt::algorithm::rs256(public_key, "", "", ""))
                        .with_issuer(kIssuer);
    verifier.verify(decoded);
    long long user_id = std::stoll(decoded.get_payload_claim("sub").as_string());
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

std::pair<std::string, std::string> generateRsaKeys(int bits) {
  RSA*    rsa = RSA_new();
  BIGNUM* e   = BN_new();
  BN_set_word(e, RSA_F4);
  RSA_generate_key_ex(rsa, bits, e, nullptr);

  EVP_PKEY* pkey = EVP_PKEY_new();
  EVP_PKEY_assign_RSA(pkey, rsa);

  BIO* priv_bio = BIO_new(BIO_s_mem());
  if (!PEM_write_bio_PrivateKey(priv_bio, pkey, nullptr, nullptr, 0, nullptr, nullptr)) {
    BIO_free_all(priv_bio);
    EVP_PKEY_free(pkey);
    BN_free(e);
    throw std::runtime_error("Failed to write PKCS#8 private key");
  }

  BUF_MEM* priv_ptr;
  BIO_get_mem_ptr(priv_bio, &priv_ptr);
  std::string private_key(priv_ptr->data, priv_ptr->length);

  BIO* pub_bio = BIO_new(BIO_s_mem());
  PEM_write_bio_PUBKEY(pub_bio, pkey);
  BUF_MEM* pub_ptr;
  BIO_get_mem_ptr(pub_bio, &pub_ptr);
  std::string public_key(pub_ptr->data, pub_ptr->length);

  BIO_free_all(priv_bio);
  BIO_free_all(pub_bio);
  EVP_PKEY_free(pkey);
  BN_free(e);

  return {private_key, public_key};
}

}  // namespace JwtUtils
