#ifndef BACKEND_NOTIFICATIONSERVICE_SERVER_SERVER_H_
#define BACKEND_NOTIFICATIONSERVICE_SERVER_SERVER_H_

#include <crow.h>

class ISocket;
class SocketHandlersRepository;
class IActiveSocketRepository;
class ISubscriber;

using SocketPtr = std::shared_ptr<ISocket>;

class Server {
 public:
  Server(int port, IActiveSocketRepository* active_socket_repository,
         SocketHandlersRepository* socket_handlers_repository, ISubscriber* subscriber);
  void run();

 protected:
  void handleSocketOnMessage(const SocketPtr& socket, const std::string& data);

 private:
  void initRoutes();
  void handleSocketRoutes();

  crow::SimpleApp app_;
  ISubscriber* subscriber_;
  IActiveSocketRepository* active_sockets_;
  SocketHandlersRepository* socket_handlers_repository_;
  const int notification_port_;
};

#endif  // BACKEND_NOTIFICATIONSERVICE_SERVER_SERVER_H_
