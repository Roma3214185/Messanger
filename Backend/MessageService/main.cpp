#include <QCoreApplication>
#include <QDebug>
#include <QtWebSockets>

#include "MessageDataBase/messagedatabase.h"
#include "Controller/controller.h"

const int MESSAGE_PORT = 8082;
const int CHAT_SERVICE = 8081;

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    DataBase db;
    //db.clearDataBase();

    crow::SimpleApp app;
    Controller controller(app, db);
    controller.handleRoutes();

    qDebug() << "[INFO] 8082 PORT IS SETTED";
    app.port(MESSAGE_PORT).multithreaded().run();
    return a.exec();
}


