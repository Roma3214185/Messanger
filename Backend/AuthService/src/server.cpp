#include "authservice/server.h"

#include "Debug_profiling.h"
#include "authservice/authmanager.h"
#include "authservice/interfaces/IGenerator.h"

Server::Server(crow::SimpleApp& app, int port, AuthController* controller, IGenerator* generator)
    : app_(app), port_(port), controller_(controller), generator_(generator) {
  initRoutes(); //todo(roma): remove from here
}

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

bool Server::generateKeys() { return generator_->generateKeys(); }

void Server::handleFindById() {
  CROW_ROUTE(app_, "/users/<string>")
      .methods(crow::HTTPMethod::GET)(
          [this](const crow::request& req, crow::response& res, const std::string& user_id_str) {
            PROFILE_SCOPE("/users/id " + user_id_str);
            long long user_id;
            try {
              user_id = std::stoll(user_id_str);
            } catch (...) {
              LOG_ERROR("Error whyle stoll in handleFindById");
              res.code = 400;
              res.body = "Invalid user_id";
              return;
            }
            controller_->findById(req, user_id, res);
            LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
          });
}

void Server::handleFindByTag() {
  CROW_ROUTE(app_, "/users/search")
      .methods(crow::HTTPMethod::GET)([this](const crow::request& req, crow::response& res) {
        PROFILE_SCOPE("[/users/search]");
        controller_->findByTag(req, res);
        LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
      });
}

void Server::handleRegister() {
  CROW_ROUTE(app_, "/auth/register")
      .methods(crow::HTTPMethod::Post)([this](const crow::request& req, crow::response& res) {
        PROFILE_SCOPE("/auth/register");
        controller_->registerUser(req, res);
        LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
      });
}

void Server::handleMe() {
  CROW_ROUTE(app_, "/auth/me")
      .methods("GET"_method)([this](const crow::request& req, crow::response& res) {
        PROFILE_SCOPE("/auth/me");
        controller_->handleMe(req, res);
        LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
      });
}

void Server::handleLogin() {
  CROW_ROUTE(app_, "/auth/login")
      .methods("POST"_method)([this](const crow::request& req, crow::response& res) {
        PROFILE_SCOPE("/auth/login");
        controller_->loginUser(req, res);
        LOG_INFO("Response code: {} | Body: {}", res.code, res.body);
      });
}
