#ifndef AUTH_SERVICE_SERVER_H
#define AUTH_SERVICE_SERVER_H

#include "authservice/authcontroller.h"

class Server {
 public:
  Server(crow::SimpleApp& app, int port, AuthController* controller);
  void run();

 private:
  void generateKeys();
  void initRoutes();
  void handleFindById();
  void handleFindByTag();
  void handleRegister();
  void handleMe();
  void handleLogin();

  crow::SimpleApp&   app_;
  int               port_;
  AuthController*   controller_;
};

#endif  // AUTH_SERVICE_SERVER_H
