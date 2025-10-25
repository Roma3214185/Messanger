#ifndef SERVER_H
#define SERVER_H

#include <crow/crow.h>
class NotificationManager;

class Server
{
public:

    Server(int port, NotificationManager& notifManager);

    void run();

private:
    void initRoutes();
    void handleSocketRoutes();
    void handleSocketOnMessage(crow::websocket::connection& conn, const std::string& data, bool is_binary);

    crow::SimpleApp app_;
    NotificationManager& notifManager;
    int notificationPort;
};

#endif // SERVER_H
