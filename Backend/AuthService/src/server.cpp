#include "server.h"

#include "Debug_profiling.h"
#include "authmanager.h"

Server::Server(int port, AuthManager* manager)
    : port_(port)
    , manager_(manager)
{
    auth_controller_ = std::make_unique<AuthController>(app_, manager_);
    initRoutes();
    //generateKeys();
}

void Server::run() {
    LOG_INFO( "Starting Auth Server on port '{}'", port_);
    app_.port(port_).multithreaded().run();
}

void Server::initRoutes() {
  handleFindById();
  handleFindByTag();
  handleRegister();
  handleMe();
  handleLogin();
}

void Server::generateKeys() {
  auth_controller_->generateKeys();
}

void Server::handleFindById() {
  CROW_ROUTE(app_, "/users/<int>")
    .methods(crow::HTTPMethod::GET)(
      [this](const crow::request& req, crow::response& res, int user_id) {
          PROFILE_SCOPE("/users/id " + std::to_string(user_id));
          auth_controller_->findById(req, user_id, res);
          LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
      });
}

void Server::handleFindByTag() {
  CROW_ROUTE(app_, "/users/search")
  .methods(crow::HTTPMethod::GET)([this](const crow::request& req, crow::response& res) {
    PROFILE_SCOPE("[/users/search]");
    auto tag = req.url_params.get("tag");
    auth_controller_->findByTag(req, tag, res);
    LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
  });
}

void Server::handleRegister() {
  CROW_ROUTE(app_, "/auth/register")
  .methods(crow::HTTPMethod::Post)([this](const crow::request& req, crow::response& res) {
    PROFILE_SCOPE("/auth/register");
    auth_controller_->registerUser(req, res);
    LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
  });
}

void Server::handleMe() {
  CROW_ROUTE(app_, "/auth/me")
    .methods("GET"_method)([this](const crow::request& req, crow::response& res)  {
    PROFILE_SCOPE("/auth/me");
    auth_controller_->handleMe(req, res);
    LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
  });
}

void Server::handleLogin() {
  CROW_ROUTE(app_, "/auth/login")
  .methods("POST"_method)([this](const crow::request& req, crow::response& res) {
    PROFILE_SCOPE("/auth/login");
    auth_controller_->loginUser(req, res);
    LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
  });
}
