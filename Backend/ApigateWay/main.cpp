#include <crow.h>
#include <cstdlib>
#include <iostream>
#include <string>
#include <chrono>
#include "src/AuthVerifier.h"
#include "src/ratelimiter.h"
#include "src/proxyclient.h"

using json = nlohmann::json;
using namespace std::chrono_literals;


static std::string getenv_or(const char* key, const char* def) {
    const char* v = std::getenv(key);
    return v ? std::string(v) : std::string(def);
}

std::string getMethod(const crow::HTTPMethod& method){
    return method == crow::HTTPMethod::GET ? "GET" : (method == crow::HTTPMethod::Delete ? "DELETE" : (method == crow::HTTPMethod::Put ? "PUT" : "POST"));
}

int main(int argc, char** argv) {
    crow::SimpleApp app;

    const std::string AUTH_SERVICE_URL = getenv_or("AUTH_SERVICE_URL", "http://localhost:8083");
    const std::string СHAT_SERVICE_URL = getenv_or("PRODUCT_SERVICE_URL", "http://localhost:8081");
    const std::string MESSANGE_SERVICE_URL = getenv_or("ORDER_SERVICE_URL", "http://localhost:8082");
    const std::string NOTIFICATION_SERVICE_URL = getenv_or("PAYMENT_SERVICE_URL", "http://localhost:8080");

    const int PORT = std::stoi(getenv_or("PORT", "8084"));

    RateLimiter rateLimiter(300, std::chrono::seconds(900));

    ProxyClient authProxy(AUTH_SERVICE_URL);
    ProxyClient chatProxy(СHAT_SERVICE_URL);
    ProxyClient messangeProxy(MESSANGE_SERVICE_URL);
    ProxyClient notificationProxy(NOTIFICATION_SERVICE_URL);

    AuthVerifier verifier(AUTH_SERVICE_URL);

    CROW_ROUTE(app, "/healthz")([
    ](const crow::request&, crow::response& res){
        json info = {
            {"status", "ok"},
            {"timestamp", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()}
        };
        res.set_header("Content-Type", "application/json");
        res.write(info.dump());
        res.end();
    });

    auto proxy_handler = [&](ProxyClient &proxy, const std::string& downstream_base_path, bool require_auth){
        return [&](const crow::request &req, crow::response &res){
            // rate limit per remote addr
            std::string ip = req.remote_ip_address;
            if (!rateLimiter.allow(ip)) {
                res.code = 429;
                res.write("Rate limit exceeded");
                res.end();
                return;
            }

            std::vector<std::pair<std::string,std::string>> extra_headers;

            if (require_auth) {
                auto authHeader = req.get_header_value("Authorization");
                if (authHeader.empty()) {
                    res.code = 401; res.write("Missing Authorization"); res.end(); return;
                }

                std::string token;
                if (authHeader.rfind("Bearer ", 0) == 0) token = authHeader.substr(7);
                else token = authHeader;
                auto v = verifier.verify(token);
                if (!v.first) { res.code = 401; res.write("Invalid token"); res.end(); return; }
                extra_headers.emplace_back("X-User-Info", v.second);
            }

            std::string path = req.url;
            // if downstream_base_path is not empty, replace prefix
            if (!downstream_base_path.empty()) {
                if (req.url.rfind(downstream_base_path, 0) == 0) path = req.url.substr(downstream_base_path.size());
                if (path.empty()) path = "/";
            }

            auto result = proxy.forward(req, path, getMethod(req.method));
            res.code = result.first;
            res.set_header("Content-Type", "application/json");
            res.write(result.second);
            res.end();
        };
    };


    CROW_ROUTE(app, "/auth/<path>")
    .methods(crow::HTTPMethod::GET, crow::HTTPMethod::POST)
    ([&](const crow::request& req, crow::response& res, std::string path){

        crow::request r = req;
        std::string downstream_path = "/auth/" + path;
        auto result = authProxy.forward(req, downstream_path, getMethod(req.method));
        res.code = result.first; res.write(result.second); res.end();
    });


    CROW_ROUTE(app, "/chats/<path>")([&](const crow::request& req, crow::response& res, std::string path){

        std::string authHeader = req.get_header_value("Authorization");
        if (authHeader.empty()) { res.code = 401; res.write("Missing Authorization"); res.end(); return; }

        std::string token = (authHeader.rfind("Bearer ",0)==0)?authHeader.substr(7):authHeader;
        auto v = verifier.verify(token);

        if (!v.first) { res.code = 401; res.write("Invalid token"); res.end(); return; }

        std::vector<std::pair<std::string,std::string>> extra = {{"X-User-Info", v.second}}; //??
        std::string downstream_path = "/chats/" + path;
        auto result = chatProxy.forward(req, downstream_path, getMethod(req.method));
        res.code = result.first; res.write(result.second); res.end();
    });

    CROW_ROUTE(app, "/messages/<path>")([&](const crow::request& req, crow::response& res, std::string path){
        std::string authHeader = req.get_header_value("Authorization");
        if (authHeader.empty()) { res.code = 401; res.write("Missing Authorization"); res.end(); return; }
        std::string token = (authHeader.rfind("Bearer ",0)==0)?authHeader.substr(7):authHeader;
        auto v = verifier.verify(token);
        if (!v.first) { res.code = 401; res.write("Invalid token"); res.end(); return; }
        std::vector<std::pair<std::string,std::string>> extra = {{"X-User-Info", v.second}}; //??
        std::string downstream_path = "/messages/" + path;
        auto result = messangeProxy.forward(req, downstream_path, getMethod(req.method));
        res.code = result.first; res.write(result.second); res.end();
    });

    CROW_ROUTE(app, "/notification/<path>")([&](const crow::request& req, crow::response& res, std::string path){
        std::string downstream_path = "/notification/" + path;
        auto result = notificationProxy.forward(req, downstream_path, getMethod(req.method));
        res.code = result.first; res.write(result.second); res.end();
    });

    CROW_ROUTE(app, "/users/<path>")([&](const crow::request& req, crow::response& res, std::string path){
        std::string downstream_path = "/users/" + path;
        auto result = authProxy.forward(req, downstream_path, getMethod(req.method));
        res.code = result.first; res.write(result.second); res.end();
    });

    //CROW_ROUTE(app_, "/users/search").methods(crow::HTTPMethod::GET)(


    std::cout << "Starting API Gateway on port " << PORT << "\n";
    app.port(PORT).multithreaded().run();
    return 0;
}
