#include "src/AuthController/authcontroller.h"
#include "src/AuthManager/authmanager.h"
#include "src/DataBase/database.h"
#include <iostream>

const int AUTH_PORT = 8083;

int main(){
    DataBase bd("mydatabase.db");
    UserRepository userRepo(bd);
    AuthManager manager(userRepo);
    crow::SimpleApp app;
    //userRepo.clear();

    AuthController controller (app, &manager);
    controller.initRoutes();

    app.port(AUTH_PORT).multithreaded().run();
    return 0;
}
