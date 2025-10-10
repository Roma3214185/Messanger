#include <crow.h>
#include "src/DataBase/database.h"
#include "src/Controller/controller.h"
#include <QCoreApplication>

const int CHAT_SERVICE_PORT = 8081;

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    crow::SimpleApp app;
    DataBase db;
    //db.clearDataBase();

    Controller controller(app, db);
    controller.handleRoutes();

    app.port(CHAT_SERVICE_PORT).multithreaded().run();
}


