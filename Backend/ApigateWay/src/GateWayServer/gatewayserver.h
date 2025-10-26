#ifndef GATEWAYSERVER_H
#define GATEWAYSERVER_H

#include <string>
#include <crow.h>

#include "Headers/AuthVerifier.h"
#include "Headers/ratelimiter.h"
#include "ProxyClient/proxyclient.h"

class GatewayServer
{

public:

    explicit GatewayServer(const int& port);
    void run();

private:

    crow::SimpleApp app_;
    int port_;

    RateLimiter rateLimiter_;
    AuthVerifier authVerifier_;
    ProxyClient authProxy_;
    ProxyClient chatProxy_;
    ProxyClient messageProxy_;
    ProxyClient notificationProxy_;

    void registerRoutes();
    void registrerHealthCheck();
    void registerNotificationRoutes();
    void registerUserRoutes();
    void registerMessagesRoutes();
    void registerChatRoutes();
    void registerAuthRoutes();

    bool checkRateLimit(const crow::request& req, crow::response& res);
    std::string getMethod(const crow::HTTPMethod& method) const;
    std::string extractToken(const crow::request& req) const;
};

#endif // GATEWAYSERVER_H
