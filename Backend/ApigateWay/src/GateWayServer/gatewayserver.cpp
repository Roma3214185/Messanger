#include "gatewayserver.h"
#include <cstdlib>
#include <iostream>
#include <chrono>

using json = nlohmann::json;
using namespace std::chrono_literals;
using std::string;

static string getenv_or(const char* key, const char* def) {
    const char* v = std::getenv(key);
    return v ? string(v) : string(def);
}

GatewayServer::GatewayServer(const int& port)
    : port_(port)
    , rateLimiter_(300, std::chrono::seconds(900))
    , authVerifier_(getenv_or("AUTH_SERVICE_URL", "http://localhost:8083"))
    , authProxy_(getenv_or("AUTH_SERVICE_URL", "http://localhost:8083"))
    , chatProxy_(getenv_or("PRODUCT_SERVICE_URL", "http://localhost:8081"))
    , messageProxy_(getenv_or("ORDER_SERVICE_URL", "http://localhost:8082"))
    , notificationProxy_(getenv_or("PAYMENT_SERVICE_URL", "http://localhost:8080"))
{
    registerRoutes();
}

void GatewayServer::run() {
    std::cout << "Starting API Gateway on port " << port_ << "\n";
    app_.port(port_).multithreaded().run();
}

string GatewayServer::getMethod(const crow::HTTPMethod& method) const {
    switch (method) {
    case crow::HTTPMethod::GET: return "GET";
    case crow::HTTPMethod::Delete: return "DELETE";
    case crow::HTTPMethod::Put: return "PUT";
    default: return "POST";
    }
}

string GatewayServer::extractToken(const crow::request& req) const {
    string authHeader = req.get_header_value("Authorization");
    if (authHeader.empty()) return {};
    return (authHeader.rfind("Bearer ", 0) == 0) ? authHeader.substr(7) : authHeader;
}

bool GatewayServer::checkRateLimit(const crow::request& req, crow::response& res) {
    string ip = req.remote_ip_address;
    if (!rateLimiter_.allow(ip)) {
        res.code = 429;
        res.write("Rate limit exceeded");
        res.end();
        return false;
    }
    return true;
}

void GatewayServer::registerRoutes() {
    registrerHealthCheck();
    registerNotificationRoutes();
    registerUserRoutes();
    registerMessagesRoutes();
    registerChatRoutes();
    registerAuthRoutes();
}

void GatewayServer::registerAuthRoutes(){
    CROW_ROUTE(app_, "/auth/<path>")
        .methods(crow::HTTPMethod::GET, crow::HTTPMethod::POST)
        ([this](const crow::request& req, crow::response& res, string path){
            string downstream_path = "/auth/" + path;
            auto result = authProxy_.forward(req, downstream_path, getMethod(req.method));
            res.code = result.first;
            res.write(result.second);
            res.end();
        });
}

void GatewayServer::registerChatRoutes(){
    CROW_ROUTE(app_, "/chats/<path>")
    ([this](const crow::request& req, crow::response& res, string path){
        if (!checkRateLimit(req, res)) return;

        string token = extractToken(req);
        auto v = authVerifier_.verify(token);
        if (!v.first) { res.code = 401; res.write("Invalid token"); res.end(); return; }

        string downstream_path = "/chats/" + path;
        auto result = chatProxy_.forward(req, downstream_path, getMethod(req.method));
        res.code = result.first;
        res.write(result.second);
        res.end();
    });
}

void GatewayServer::registerMessagesRoutes(){
    CROW_ROUTE(app_, "/messages/<path>")
    ([this](const crow::request& req, crow::response& res, string path){
        if (!checkRateLimit(req, res)) return;

        string token = extractToken(req);
        auto v = authVerifier_.verify(token);
        if (!v.first) { res.code = 401; res.write("Invalid token"); res.end(); return; }

        string downstream_path = "/messages/" + path;
        auto result = messageProxy_.forward(req, downstream_path, getMethod(req.method));
        res.code = result.first;
        res.write(result.second);
        res.end();
    });
}

void GatewayServer::registerNotificationRoutes(){
    CROW_ROUTE(app_, "/notification/<path>")
    ([this](const crow::request& req, crow::response& res, string path){
        string downstream_path = "/notification/" + path;
        auto result = notificationProxy_.forward(req, downstream_path, getMethod(req.method));
        res.code = result.first;
        res.write(result.second);
        res.end();
    });
}

void GatewayServer::registerUserRoutes(){
    CROW_ROUTE(app_, "/users/<path>")
    ([this](const crow::request& req, crow::response& res, string path){
        string downstream_path = "/users/" + path;
        auto result = authProxy_.forward(req, downstream_path, getMethod(req.method));
        res.code = result.first;
        res.write(result.second);
        res.end();
    });
}

void GatewayServer::registrerHealthCheck(){
    CROW_ROUTE(app_, "/healthz")([](const crow::request&, crow::response& res){
        json info = {
            {"status", "ok"},
            {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::system_clock::now().time_since_epoch()
                              ).count()}
        };
        res.set_header("Content-Type", "application/json");
        res.write(info.dump());
        res.end();
    });
}
