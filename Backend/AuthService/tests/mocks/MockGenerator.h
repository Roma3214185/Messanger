#ifndef MOCKGENERATOR_H
#define MOCKGENERATOR_H

#include "authservice/interfaces/IGenerator.h"

class MockGenerator : public IGenerator {
  public:
    int last_user_id;
    std::string mock_token;

    bool generateKeys() {

    }

    std::string generateToken(int user_id) {
      last_user_id = user_id;
      return mock_token;
    }
};

#endif // MOCKGENERATOR_H
