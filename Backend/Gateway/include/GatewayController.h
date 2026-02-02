#ifndef GATEWAYCONTROLLER_H
#define GATEWAYCONTROLLER_H

#include "proxyclient.h"

class IEventBus;
class ICacheService;
class IThreadPool;
class IClient;
class IMetrics;

class GatewayController {
    ProxyClient proxy_;
    ICacheService *cache_;
    IThreadPool *pool_;
    IEventBus *queue_;
public:

    GatewayController(IClient *client, ICacheService *cache, IThreadPool *pool, IEventBus *queue);

    void handleProxyRequest(const crow::request &req, crow::response &res, const int port,
                            const std::string &path);

    void handlePostRequest(
        const crow::request &req,  // todo: make handlers and unordered_map<request, handler>
        crow::response &res, const int port, const std::string &path);

    void handleRequestRoute(crow::response& res, std::string task_id);

    void subscribeOnNewRequest();
};

#endif // GATEWAYCONTROLLER_H
