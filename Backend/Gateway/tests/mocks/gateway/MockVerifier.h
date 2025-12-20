#ifndef MOCKVERIFIER_H
#define MOCKVERIFIER_H

#include "interfaces/IVerifier.h"

class MockVerifier : public IVerifier {
  public:
    std::string last_token;
    int call_verify = 0;
    std::optional<long long> mock_ans;

    std::optional<long long> verifyTokenAndGetUserId(const std::string& token) {
      last_token = token;
      ++call_verify;
      return mock_ans;
    }
};

#endif // MOCKVERIFIER_H
