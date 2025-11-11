#ifndef AUTHCONTROLLER_H
#define AUTHCONTROLLER_H

#include <crow.h>

#include "authmanager.h"

class AuthController {
 public:
  AuthController(crow::SimpleApp& app, AuthManager* service);
  void generateKeys();
  void findById(const crow::request& req, int user_id, crow::response& responce);
  void findByTag(const crow::request& req, const std::string& tag, crow::response& responce);
  void registerUser(const crow::request& req, crow::response& responce);
  void handleMe(const crow::request& req, crow::response& responce);
  void loginUser(const crow::request& req, crow::response& responce);

 private:
  void handleRegister();
  void handleLogin();

  std::optional<AuthResponce> verifyToken(const crow::request& req);

  crow::SimpleApp& app_;
  AuthManager*     service_;
};

#endif  // AUTHCONTROLLER_H
