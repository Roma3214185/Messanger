#ifndef SERVER_H
#define SERVER_H

#include "AuthManager/authmanager.h"
#include "AuthController/authcontroller.h"

using AuthControllerPtr = std::unique_ptr<AuthController>;

class Server {
public:
    Server(const int& port, AuthManager* manager);
    void run();

private:
    void generateKeys();
    void initRoutes();
    crow::SimpleApp app;
    AuthManager* manager;
    int port_;
    AuthControllerPtr authController;
};

#endif // SERVER_H
