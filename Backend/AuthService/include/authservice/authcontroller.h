#ifndef AUTHCONTROLLER_H
#define AUTHCONTROLLER_H

#include "ProdConfigProvider.h"

class IAuthManager;
class AuthResponce;
class IAutoritizer;
class IGenerator;
class RequestDTO;

using StatusCode = int;
using ResponseBody = std::string;
using Response = std::pair<StatusCode, ResponseBody>;

class AuthController {
 public:
  using OptionalId = std::optional<long long>;
  using Token = std::string;

  AuthController(IAuthManager* manager, IAutoritizer* authoritizer, IGenerator* generator,
                 IConfigProvider* provider = &ProdConfigProvider::instance());
  Response findById(const RequestDTO& req, const std::string& user_id_str);
  Response findByTag(const RequestDTO& req);
  Response registerUser(const RequestDTO& req);
  Response handleMe(const RequestDTO& req);
  Response loginUser(const RequestDTO& req);
  bool generateKeys();

 private:
  OptionalId verifyToken(const std::string& token);

  IAuthManager* manager_;
  IConfigProvider* provider_;
  IAutoritizer* authoritizer_;
  IGenerator* generator_;
};

#endif  // AUTHCONTROLLER_H
