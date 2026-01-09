#ifndef JWTGENERATOR_H
#define JWTGENERATOR_H

#include <fstream>

#include "JwtUtils.h"
#include "interfaces/IGenerator.h"

class JwtGenerator : public IGenerator {
 public:
  bool generateKeys() override {
    const std::string kKeysDir = "/Users/roma/QtProjects/Chat/Backend/shared_keys/";
    const std::string kPtivateKeysDir = "/Users/roma/QtProjects/Chat/Backend/AuthService/private_keys/";
    const std::string kPrivateKeyFile = kPtivateKeysDir + "private_key.pem";
    const std::string kPublicKeyFile = kKeysDir + "public_key.pem";

    auto [private_key, public_key] = JwtUtils::generateRsaKeys();
    std::filesystem::create_directories(kKeysDir);

    try {
      saveInFile(kPrivateKeyFile, private_key);
      saveInFile(kPublicKeyFile, public_key);

      LOG_INFO("Save private_key in {}: {}", kPrivateKeyFile, private_key);
      LOG_INFO("Save public_key in {}: {}", kPublicKeyFile, public_key);
      return true;
    } catch (...) {
      LOG_ERROR("Error saving keys");
      return false;
    }
  }

  std::string generateToken(long long user_id) override { return JwtUtils::generateToken(user_id); }

 private:
  void saveInFile(const std::string &file_name, const std::string &key) {
    std::ofstream file(file_name);
    if (!file) throw std::runtime_error("Cannot open file for writing");
    file << key;
    file.close();
  }
};

#endif  // JWTGENERATOR_H
