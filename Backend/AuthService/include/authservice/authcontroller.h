#ifndef AUTHCONTROLLER_H
#define AUTHCONTROLLER_H

#include <crow.h>
#include "ProdConfigProvider.h"

class IAuthManager;
class AuthResponce;
class IAutoritizer;

class AuthController {
 public:
  using OptionalId = std::optional<long long>;
  using Token = std::string;

  AuthController(IAuthManager* service, IAutoritizer* authoritier, IConfigProvider* provider = &ProdConfigProvider::instance());
  void generateKeys();
  void findById(const crow::request& req, int user_id, crow::response& responce);
  void findByTag(const crow::request& req, crow::response& responce);
  void registerUser(const crow::request& req, crow::response& responce);
  void handleMe(const crow::request& req, crow::response& responce);
  void loginUser(const crow::request& req, crow::response& responce);

 private:
  void handleRegister();
  void handleLogin();
  Token generateToken(int user_id);
  std::pair<OptionalId, Token> verifyToken(const crow::request& req);

  IAuthManager* manager_;
  IConfigProvider* provider_;
  IAutoritizer* authoritizer_;
};

#endif  // AUTHCONTROLLER_H
