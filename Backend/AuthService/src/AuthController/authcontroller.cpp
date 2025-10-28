#include "authcontroller.h"

#include <string>
#include <utility>

#include "Debug_profiling.h"
#include "RegisterRequest.h"

using std::string;

constexpr int kUserError = 400;
constexpr int kSuccessfulCode = 200;

namespace {

crow::json::wvalue userToJson(const User& user, const std::string& token = "") {
  crow::json::wvalue res;
  if (!token.empty()) {
    res["token"] = token;
  }
  res["user"]["id"] = user.id;
  res["user"]["email"] = user.email;
  res["user"]["name"] = user.username;
  res["user"]["tag"] = user.tag;

  LOG_INFO(
      "[user][id] = '{}' | [email] = '{}' | "
      "[name] = '{}' | [tag] = '{}', token = '{}'",
      user.id, user.username, user.email, user.tag, token);
  return res;
}

}  // namespace

AuthController::AuthController(crow::SimpleApp& app, AuthManager* service)
    : app_(app), service_(service) {}

void AuthController::initRoutes() {
  handleLogin();
  handleRegister();
  handleMe();
  handleFindByTag();
  handleFindById();
  spdlog::debug("[authcontroller] routes inited");
}

void AuthController::handleLogin() {
  CROW_ROUTE(app_, "/auth/login")
      .methods("POST"_method)([this](const crow::request& req) -> crow::response {
        PROFILE_SCOPE("/auth/login");
        auto body = crow::json::load(req.body);
        if (!body) {
          LOG_ERROR("[Login] Invalid Json");
          return crow::response(kUserError, "Invalid JSON");
        }
        LoginRequest login_request{
            .email = body["email"].s(),
            .password = body["password"].s(),
        };
        LOG_INFO("[login] user email: '{}' and user password '{}'",
                 login_request.email, login_request.password);

        auto authRes = service_->loginUser(login_request);

        if (!authRes) {
          LOG_ERROR("[login] invalid credentials");
          return crow::response(kUserError, "Invalid credentials");
        }
        LOG_ERROR("[login] successfull: name '{}' | id '{}' | tag '{}'",
                  (*authRes->user).username, (*authRes->user).id,
                  (*authRes->user).tag);
        return crow::response(kSuccessfulCode, userToJson(*authRes->user, authRes->token));
      });
}

void AuthController::handleMe() {
  CROW_ROUTE(app_, "/auth/me")
      .methods("GET"_method)([this](const crow::request& req) -> crow::response  {
        PROFILE_SCOPE("/auth/me");
        auto authRes = verifyToken(req);
        if (!authRes) {
          LOG_ERROR("Ivalid or expired token");
          return crow::response(kUserError, "Invalid or expired token");
        }
        return crow::response(userToJson(*authRes->user, authRes->token));
      });
}

void AuthController::handleRegister() {
  CROW_ROUTE(app_, "/auth/register")
      .methods(crow::HTTPMethod::Post)([this](const crow::request& req) -> crow::response  {
        PROFILE_SCOPE("/auth/register");
        auto body = crow::json::load(req.body);
        if (!body) {
          LOG_ERROR("[register] invalid json");
          return crow::response(kUserError, "Invalid JSON");
        }

        auto regReq = RegisterRequest{.email = body["email"].s(),
                                      .password = body["password"].s(),
                                      .name = body["name"].s(),
                                      .tag = body["tag"].s()};

        auto authRes = service_->registerUser(regReq);
        if (!authRes) {
          LOG_ERROR("[register] User already exists");
          return crow::response(kUserError, "User already exists");
        }

        LOG_INFO("[register] user (id='{}' ", authRes->user->id);
        return crow::response(userToJson(*authRes->user, authRes->token));
      });
}

void AuthController::handleFindByTag() {
  CROW_ROUTE(app_, "/users/search")
      .methods(crow::HTTPMethod::GET)([this](const crow::request& req) -> crow::response  {
        PROFILE_SCOPE("[/users/search]");
        auto tag = req.url_params.get("tag");
        if (!tag) {
          LOG_ERROR("Missing tag parametr");
          return crow::response(kUserError, "Missing 'tag' parameter");
        }

        auto listOfUsers = service_->findUserByTag(tag);
        LOG_INFO("With tag '{}' was finded '{}' users", tag,
                 listOfUsers.size());

        crow::json::wvalue res;
        res["users"] = crow::json::wvalue::list();

        size_t idx = 0;
        for (const auto& user : listOfUsers) {
          auto userJson = userToJson(user);
          res["users"][idx++] = std::move(userJson["user"]);
        }

        return crow::response(kSuccessfulCode, res);
      });
}

void AuthController::handleFindById() {
  CROW_ROUTE(app_, "/users/<int>")
      .methods(crow::HTTPMethod::GET)(
          [this](const crow::request& req, int user_id) -> crow::response  {
            PROFILE_SCOPE("/users/id");
            auto foundUser = service_->findUserById(user_id);
            if (!foundUser) {
              LOG_ERROR("[handleById] User not found with id '{}'", user_id);
              return crow::response{kUserError, "Users not found"};
            }

            LOG_INFO("[handleById] User found with id '{}'", user_id);
            auto userJson = userToJson(*foundUser);
            return crow::response(kSuccessfulCode, userJson["user"]);
          });
}

std::optional<AuthResponce> AuthController::verifyToken(
    const crow::request& req) {
  auto token = req.get_header_value("Authorization");
  if (token.empty()) {
    LOG_ERROR("Token empty");
    return std::nullopt;
  }

  return service_->getUser(token);
}
