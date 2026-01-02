#ifndef AUTH_SERVICE_SERVER_H
#define AUTH_SERVICE_SERVER_H

#include "authservice/authcontroller.h"

class IGenerator;

class Server {
 public:
  Server(crow::SimpleApp& app, int port, AuthController* controller, IGenerator* generator);
  void initRoutes();
  void run();
  [[nodiscard]] bool generateKeys();

 private:
  void handleFindById();
  void handleFindByTag();
  void handleRegister();
  void handleMe();
  void handleLogin();

  crow::SimpleApp&   app_;
  int               port_;
  AuthController*   controller_;
  IGenerator* generator_;
};

#endif  // AUTH_SERVICE_SERVER_H
