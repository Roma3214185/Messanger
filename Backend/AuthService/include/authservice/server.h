#ifndef AUTH_SERVICE_SERVER_H
#define AUTH_SERVICE_SERVER_H

#include "authservice/authcontroller.h"

class AuthManager;

using AuthControllerPtr = std::unique_ptr<AuthController>;

class Server {
 public:
  Server(int port, AuthManager* manager);
  void run();

 private:
  void generateKeys();
  void initRoutes();
  void handleFindById();
  void handleFindByTag();
  void handleRegister();
  void handleMe();
  void handleLogin();

  crow::SimpleApp   app_;
  AuthManager*      manager_;
  int               port_;
  AuthControllerPtr auth_controller_;
};

#endif  // AUTH_SERVICE_SERVER_H
