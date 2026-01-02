#ifndef BACKEND_NOTIFICATIONSERVICE_SERVER_SERVER_H_
#define BACKEND_NOTIFICATIONSERVICE_SERVER_SERVER_H_

#include <crow.h>
#include "interfaces/IMessageHandler.h"
#include "notificationservice/SocketRepository.h"

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

  crow::SimpleApp      app_;
  NotificationManager* notification_manager_;
  const int            notification_port_;
  std::unordered_map<std::string, std::unique_ptr<IMessageHandler>> handlers_;
  SocketRepository active_sockets_;
};

#endif  // BACKEND_NOTIFICATIONSERVICE_SERVER_SERVER_H_
