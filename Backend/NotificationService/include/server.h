#ifndef BACKEND_NOTIFICATIONSERVICE_SERVER_SERVER_H_
#define BACKEND_NOTIFICATIONSERVICE_SERVER_SERVER_H_

#include <crow/crow.h>

class NotificationManager;

class Server {
 public:
  Server(int port, NotificationManager& notification_manager);
  void run();

 private:
  void initRoutes();
  void handleSocketRoutes();
  void handleSocketOnMessage(crow::websocket::connection& conn,
                             const std::string&           data,
                             bool                         is_binary);

  crow::SimpleApp      app_;
  NotificationManager& notification_manager_;
  const int            notification_port_;
};

#endif  // BACKEND_NOTIFICATIONSERVICE_SERVER_SERVER_H_
