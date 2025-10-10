#ifndef AUTHCONTROLLER_H
#define AUTHCONTROLLER_H

#include <crow/crow.h>
#include "AuthManager/authmanager.h"

class AuthController
{
    crow::SimpleApp& app_;
    AuthManager* service_;
public:
    AuthController(crow::SimpleApp& app, AuthManager* service);
    void initRoutes();
private:
    void handleRegister();
    void handleLogin();
    void handleMe();
    void handleFindByTag();
    void handleFindById();
};

#endif // AUTHCONTROLLER_H
