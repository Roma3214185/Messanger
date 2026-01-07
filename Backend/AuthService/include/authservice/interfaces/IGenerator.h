#ifndef IGENERATOR_H
#define IGENERATOR_H

#include <string>

#include "entities/User.h"

class IGenerator {
public:
  virtual ~IGenerator() = default;

  virtual bool generateKeys() = 0;
  virtual std::string generateToken(long long user_id) = 0;
};

#endif // IGENERATOR_H
