#ifndef AUTHCONTROLLER_H
#define AUTHCONTROLLER_H

#include <crow/crow.h>

#include "authmanager.h"

class AuthController {
 public:
  AuthController(crow::SimpleApp& app, AuthManager* service);
  void initRoutes();

 private:
  void handleRegister();
  void handleLogin();
  void handleMe();
  void handleFindByTag();
  void handleFindById();
  std::optional<AuthResponce> verifyToken(const crow::request& req);

  crow::SimpleApp& app_;
  AuthManager* service_;
};

#endif  // AUTHCONTROLLER_H
