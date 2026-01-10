#include "authservice/authcontroller.h"

#include <string>
#include <utility>

#include "Debug_profiling.h"
#include "authservice/interfaces/IAuthManager.h"
#include "authservice/interfaces/IGenerator.h"
#include "config/codes.h"
#include "entities/RegisterRequest.h"
#include "entities/RequestDTO.h"
#include "interfaces/IAutoritizer.h"

using std::string;

namespace {

crow::json::wvalue userToJson(const User &user, std::string_view token = {}) {
  crow::json::wvalue res;
  if (!token.empty()) {
    res["token"] = std::string(token);
  }
  res["user"]["id"] = user.id;
  res["user"]["email"] = user.email;
  res["user"]["name"] = user.username;
  res["user"]["tag"] = user.tag;
  res["user"]["avatar"] = user.avatar;

  LOG_INFO("User to return {}", res.dump());
  return res;
}

[[nodiscard]] Response sendResponse(int code, const std::string &text, bool is_error) {
  auto text_json = [&]() -> std::string { return is_error ? utils::details::formError(text) : text; };

  LOG_INFO("Return responce {} and body {}, is_error {}", code, text, std::to_string(is_error));
  return std::make_pair(code, text_json());
}

std::optional<std::string> fetchTag(const RequestDTO &req) {
  auto it = req.url_params.find("tag");
  return it != req.url_params.end() ? std::make_optional(it->second) : std::nullopt;
}

std::optional<long long> getIdFromStr(const std::string &str) {
  try {
    return std::stoll(str);
  } catch (...) {
    LOG_ERROR("Error while get stoll of {}", str);
    return std::nullopt;
  }
}

}  // namespace

AuthController::AuthController(IAuthManager *manager, IAutoritizer *authoritizer, IGenerator *generator)
    : manager_(manager), authoritizer_(authoritizer), generator_(generator) {}  // todo(roma) make itokengenrator (ISRP)

Response AuthController::loginUser(const RequestDTO &req) {
  auto body = crow::json::load(req.body);
  if (!body || !body.count("email") || !body.count("password")) {  // todo: make function bool contains(json, field)
    return sendResponse(Config::StatusCodes::badRequest, "Invalid Json", true);
  }
  LoginRequest login_request{
      .email = body["email"].s(),
      .password = body["password"].s(),
  };

  LOG_INFO("[login] user email: '{}' and user password '{}'", login_request.email, login_request.password);

  std::optional<User> logged_user = manager_->loginUser(login_request);

  if (!logged_user) {
    LOG_ERROR("Invalid credentials");
    return sendResponse(Config::StatusCodes::badRequest, "Invalid credentials", true);
  }

  auto token = generator_->generateToken(logged_user->id);
  return sendResponse(Config::StatusCodes::success, userToJson(*logged_user, token).dump(), false);
}

Response AuthController::handleMe(const RequestDTO &req) {
  auto user_id = verifyToken(req.token);
  if (!user_id.has_value()) {
    LOG_ERROR("Invalid token");
    return sendResponse(Config::StatusCodes::unauthorized, Config::IssueMessages::invalidToken, true);
  }

  std::optional<User> user = manager_->getUser(*user_id);
  if (!user) {
    LOG_ERROR("User with id {} not found", *user_id);
    return sendResponse(Config::StatusCodes::notFound, Config::IssueMessages::userNotFound, true);
  }

  return sendResponse(Config::StatusCodes::success, userToJson(*user, req.token).dump(), false);
}

Response AuthController::findByTag(const RequestDTO &req) {
  std::optional<std::string> tag = fetchTag(req);
  if (!tag) {  // TODO: fully check tag to not go to databse
    LOG_ERROR("Missing tag parametr");
    return sendResponse(Config::StatusCodes::badRequest, "Missing tag parametr", true);
  }

  auto list_of_users = manager_->findUsersByTag(*tag);
  LOG_INFO("With tag '{}' was finded '{}' users", *tag, list_of_users.size());

  crow::json::wvalue json_users;
  json_users["users"] = crow::json::wvalue::list();

  size_t idx = 0;
  for (const auto &user : list_of_users) {
    auto user_json = userToJson(user);
    json_users["users"][idx++] = std::move(user_json["user"]);
  }

  return sendResponse(Config::StatusCodes::success, json_users.dump(),
                      false);  // todo: check in function code
}

Response AuthController::findById(const RequestDTO & /*req*/, const std::string &user_id_str) {
  std::optional<long long> user_id = getIdFromStr(user_id_str);
  if (!user_id.has_value()) {
    return sendResponse(Config::StatusCodes::badRequest, "Invalid user_id", true);
  }

  auto found_user = manager_->getUser(*user_id);
  if (!found_user) {
    LOG_ERROR("User with id {} not found", *user_id);
    return sendResponse(Config::StatusCodes::notFound, "User not found", true);
  }

  auto user_json = userToJson(*found_user);
  return sendResponse(Config::StatusCodes::success, user_json["user"].dump(), false);
}

AuthController::OptionalId AuthController::verifyToken(const std::string &token) {
  return authoritizer_->autoritize(token);
}

Response AuthController::registerUser(const RequestDTO &req) {
  auto body = crow::json::load(req.body);
  if (!body || !body.count("email") || !body.count("password") || !body.count("name") || !body.count("tag")) {
    LOG_ERROR("Invalid json");
    return sendResponse(Config::StatusCodes::badRequest, "Invalid json", true);
  }

  RegisterRequest register_request;
  register_request.email = body["email"].s();
  register_request.password = body["password"].s();
  register_request.name = body["name"].s();
  register_request.tag = body["tag"].s();

  LOG_INFO("Registe user (email: {}) | (password : {}) | (name : {}) | (tag: {})", register_request.email,
           register_request.password, register_request.name, register_request.tag);

  // TODO: create middleware for check valid input??
  std::optional<User> register_user = manager_->registerUser(register_request);
  if (!register_user) {
    LOG_ERROR("User not registered");
    return sendResponse(Config::StatusCodes::userError, "User already exist", true);
  }
  LOG_INFO("User registered successfully, id is {}", register_user->id);
  std::string token = generator_->generateToken(register_user->id);
  LOG_INFO("User user {}, id {} token is {}", register_user->username, register_user->id, token);
  return sendResponse(Config::StatusCodes::success, userToJson(*register_user, token).dump(), false);
}
