#ifndef AUTHCONTROLLER_H
#define AUTHCONTROLLER_H

#include <crow.h>
#include "ProdConfigProvider.h"

class IAuthManager;
class AuthResponce;
class IAutoritizer;
class IGenerator;

class AuthController {
 public:
  using OptionalId = std::optional<long long>;
  using Token = std::string;

  AuthController(IAuthManager* service, IAutoritizer* authoritier, IGenerator* generator,
                 IConfigProvider* provider = &ProdConfigProvider::instance());
  void findById(const crow::request& req, int user_id, crow::response& responce);
  void findByTag(const crow::request& req, crow::response& responce);
  void registerUser(const crow::request& req, crow::response& responce);
  void handleMe(const crow::request& req, crow::response& responce);
  void loginUser(const crow::request& req, crow::response& responce);
  bool generateKeys();

 private:
  std::pair<OptionalId, Token> verifyToken(const crow::request& req);

  IAuthManager* manager_;
  IConfigProvider* provider_;
  IAutoritizer* authoritizer_;
  IGenerator* generator_;
};

#endif  // AUTHCONTROLLER_H
