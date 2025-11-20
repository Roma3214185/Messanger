#ifndef JWTGENERATOR_H
#define JWTGENERATOR_H

#include "interfaces/IGenerator.h"
#include "JwtUtils.h"

class JwtGenerator : public IGenerator {
  public:
    bool generateKeys() override {
      const std::string kKeysDir        = "/Users/roma/QtProjects/Chat/Backend/shared_keys/";
      const std::string kPrivateKeyFile = "private_key.pem";
      const std::string kPublicKeyFile  = kKeysDir + "public_key.pem";

      auto [private_key, public_key] = JwtUtils::generate_rsa_keys();
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

    std::string generateToken(int user_id) override {
      return JwtUtils::generateToken(user_id);
    }
  private:
    void saveInFile(const std::string& file_name, const std::string& key) {
      std::ofstream file(file_name);
      if (!file) throw std::runtime_error("Cannot open file for writing");
      file << key;
      file.close();
    }
};

#endif // JWTGENERATOR_H
