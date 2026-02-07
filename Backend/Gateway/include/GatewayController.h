#ifndef GATEWAYCONTROLLER_H
#define GATEWAYCONTROLLER_H

#include <crow.h>

#include "proxyclient.h"

class IEventBus;
class ICacheService;
class IThreadPool;
class IClient;

class GatewayController {
 public:
  GatewayController(IClient *client, ICacheService *cache, IThreadPool *pool, IEventBus *queue);

  void handleProxyRequest(const crow::request &req, crow::response &res, const int port, const std::string &path);

  void handlePostRequest(const crow::request &req, crow::response &res, const int port, const std::string &path);

  void handleRequestRoute(crow::response &res, std::string task_id);

  void subscribeOnNewRequest();

 private:
  ProxyClient proxy_;
  ICacheService *cache_;
  IThreadPool *pool_;
  IEventBus *queue_;
};

#endif  // GATEWAYCONTROLLER_H
