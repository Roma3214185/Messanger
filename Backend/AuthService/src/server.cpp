#include "authservice/server.h"

#include "Debug_profiling.h"
#include "authservice/authmanager.h"
#include "entities/RequestDTO.h"
#include "authservice/authcontroller.h"

namespace {

void sendResponse(crow::response& res, int code, std::string& body) {
  res.code = code;
  res.write(body);
  res.end();
}

}  // namespace

Server::Server(crow::SimpleApp& app, int port, AuthController* controller)
    : app_(app), port_(port), controller_(controller) { }

void Server::run() {
  LOG_INFO("Starting Auth Server on port '{}'", port_);
  app_.port(port_).multithreaded().run();
}

void Server::initRoutes() {
  handleFindByTag();
  handleFindById();
  handleRegister();
  handleMe();
  handleLogin();
}

bool Server::generateKeys() { return controller_->generateKeys(); }

void Server::handleFindById() {
  CROW_ROUTE(app_, "/users/<string>")
      .methods(crow::HTTPMethod::GET)(
          [this](const crow::request& req, crow::response& res, const std::string& user_id_str) {
            PROFILE_SCOPE();
            auto [code, body] = controller_->findById(utils::getDTO(req, "/users/id"), user_id_str);
            sendResponse(res, code, body);
          });
}

void Server::handleFindByTag() {
  CROW_ROUTE(app_, "/users/search")
      .methods(crow::HTTPMethod::GET)([this](const crow::request& req, crow::response& res) {
        PROFILE_SCOPE();
        auto [code, body] = controller_->findByTag(utils::getDTO(req, "/users/search"));
        sendResponse(res, code, body);
      });
}

void Server::handleRegister() {
  CROW_ROUTE(app_, "/auth/register")
      .methods(crow::HTTPMethod::Post)([this](const crow::request& req, crow::response& res) {
        PROFILE_SCOPE();
        auto [code, body] = controller_->registerUser(utils::getDTO(req, "/auth/register"));
        sendResponse(res, code, body);
      });
}

void Server::handleMe() {
  CROW_ROUTE(app_, "/auth/me")
      .methods("GET"_method)([this](const crow::request& req, crow::response& res) {
        PROFILE_SCOPE("/auth/me");
        auto [code, body] = controller_->handleMe(utils::getDTO(req, "/auth/me"));
        sendResponse(res, code, body);
      });
}

void Server::handleLogin() {
  CROW_ROUTE(app_, "/auth/login")
      .methods("POST"_method)([this](const crow::request& req, crow::response& res) {
        PROFILE_SCOPE();
        auto [code, body] = controller_->loginUser(utils::getDTO(req, "/auth/login"));
        sendResponse(res, code, body);
      });
}
