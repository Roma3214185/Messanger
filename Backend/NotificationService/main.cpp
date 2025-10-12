#include <QCoreApplication>
#include <QDebug>
#include <event2/event.h>
#include <crow.h>

const int NOTIFICATION_PORT = 8081;

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    crow::SimpleApp app;

    CROW_ROUTE(app, "/notify").methods("POST"_method)([](const crow::request& req){
        std::cout << " New notification: " << req.body << std::endl;
        return crow::response(200, "OK");
    });

    app.port(NOTIFICATION_PORT).multithreaded().run();
}
