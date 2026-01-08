#ifndef MOCKGENERATOR_H
#define MOCKGENERATOR_H

#include "authservice/interfaces/IGenerator.h"

class MockTokenGenerator : public IGenerator {
 public:
  long long last_user_id;
  std::string mock_token;
  bool should_fail_generateKeys = false;

  bool generateKeys() override { return !should_fail_generateKeys; }

  std::string generateToken(long long user_id) override {
    last_user_id = user_id;
    return mock_token;
  }
};

#endif  // MOCKGENERATOR_H
