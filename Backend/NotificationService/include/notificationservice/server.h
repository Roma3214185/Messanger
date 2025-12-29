#ifndef BACKEND_NOTIFICATIONSERVICE_SERVER_SERVER_H_
#define BACKEND_NOTIFICATIONSERVICE_SERVER_SERVER_H_

#include <crow.h>
#include "interfaces/IMessageHandler.h"

class NotificationManager;
class ISocket;

using SocketPtr = std::shared_ptr<ISocket>;

class Server {
 public:
  Server(int port, NotificationManager* notification_manager);
  void run();

 protected:
  void handleSocketOnMessage(const SocketPtr& socket, const std::string& data);

 private:
  void initRoutes();
  void initHanlers();
  void handleSocketRoutes();
  SocketPtr findSocket(crow::websocket::connection* conn);

  crow::SimpleApp      app_;
  NotificationManager* notification_manager_;
  const int            notification_port_;
  std::unordered_map<std::string, std::unique_ptr<IMessageHandler>> handlers_;

  std::unordered_set<SocketPtr> active_sockets;
  std::mutex ws_mutex;
};

#endif  // BACKEND_NOTIFICATIONSERVICE_SERVER_SERVER_H_
