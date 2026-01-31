#ifndef JWTGENERATOR_H
#define JWTGENERATOR_H

#include <fstream>

#include "JwtUtils.h"
#include "interfaces/IGenerator.h"

class JwtGenerator : public IGenerator {
 public:
  bool generateKeys() override;
  std::string generateToken(long long user_id) override;

 private:
  void saveInFile(const std::string &file_name, const std::string &key);
};

#endif  // JWTGENERATOR_H
