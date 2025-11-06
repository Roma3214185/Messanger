#ifndef SERVER_H
#define SERVER_H

#include "authcontroller.h"

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

    crow::SimpleApp app_;
    AuthManager* manager_;
    int port_;
    AuthControllerPtr auth_controller_;
};

#endif // SERVER_H
