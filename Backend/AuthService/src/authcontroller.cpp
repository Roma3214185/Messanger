#include "authservice/authcontroller.h"

#include <string>
#include <utility>

#include "Debug_profiling.h"
#include "entities/RegisterRequest.h"
#include "authservice/interfaces/IAuthManager.h"
#include "interfaces/IAutoritizer.h"
#include "authservice/interfaces/IGenerator.h"
#include "entities/RequestDTO.h"

using std::string;

namespace {

crow::json::wvalue userToJson(const User& user, const std::string& token = "") {
  crow::json::wvalue res;
  if (!token.empty()) {
    res["token"] = token;
  }
  res["user"]["id"]    = user.id;
  res["user"]["email"] = user.email;
  res["user"]["name"]  = user.username;
  res["user"]["tag"]   = user.tag;
  res["user"]["avatar"] = user.avatar;

  LOG_INFO("User to return {}", res.dump());
  return res;
}

[[nodiscard]]
Response sendResponse(int code, const std::string& text, bool is_error) {
  auto text_json = [&]() -> std::string {
    nlohmann::json res_text;
    if(is_error) {
      res_text["error"] = text;
      return res_text.dump();
    } else {
      return text;
    }
  };
  LOG_INFO("Return responce {} and body {}, is_error {}", code, text, std::to_string(is_error));
  return std::make_pair(code, text_json());
}

std::optional<std::string> fetchTag(const RequestDTO& req) {
  auto it = req.url_params.find("tag");
  return it != req.url_params.end()
             ? std::make_optional(it->second)
             : std::nullopt;
}

std::optional<long long> getIdFromStr(const std::string& str) {
  try {
    return std::stoll(str);
  } catch(...) {
    LOG_ERROR("Error while get stoll of {}", str);
    return std::nullopt;
  }
}

}  // namespace

AuthController::AuthController(IAuthManager* manager, IAutoritizer* authoritizer, IGenerator* generator, IConfigProvider* provider)
    : manager_(manager), authoritizer_(authoritizer), generator_(generator), provider_(provider) {} //todo(roma) make itokengenrator (ISRP)

Response AuthController::loginUser(const RequestDTO& req) {
  auto body = crow::json::load(req.body);
  if (!body || !body.count("email") || !body.count("password")) {  //todo: make function bool contains(json, field)
    return sendResponse( provider_->statusCodes().badRequest, "Invalid Json", true);
  }
  LoginRequest login_request {
      .email    = body["email"].s(),
      .password = body["password"].s(),
  };

  LOG_INFO("[login] user email: '{}' and user password '{}'",
           login_request.email,
           login_request.password);

  std::optional<User> logged_user = manager_->loginUser(login_request);

  if (!logged_user) {
    LOG_ERROR("Invalid credentials");
    return sendResponse( provider_->statusCodes().unauthorized, "Invalid credentials", true);
  }

  auto token = generator_->generateToken(logged_user->id);
  return sendResponse( provider_->statusCodes().success, userToJson(*logged_user, token).dump(), false);
}

Response AuthController::handleMe(const RequestDTO& req) {
  auto user_id = verifyToken(req.token);
  if (!user_id) {
    LOG_ERROR("Invalid token");
    return sendResponse( provider_->statusCodes().unauthorized, provider_->issueMessages().invalidToken, true);
  }

  std::optional<User> user = manager_->getUser(*user_id);
  if(!user) {
    LOG_ERROR("User with id {} not found", *user_id);
    return sendResponse( provider_->statusCodes().notFound, provider_->issueMessages().userNotFound, true);
  }

  return sendResponse( provider_->statusCodes().success, userToJson(*user, req.token).dump(), false);
}

Response AuthController::findByTag(const RequestDTO& req) {
  std::optional<std::string> tag = fetchTag(req);
  if (!tag) { //TODO: fully check tag to not go to databse
    LOG_ERROR("Missing tag parametr");
    return sendResponse( provider_->statusCodes().badRequest, "Missing tag parametr", true);
  }

  auto list_of_users = manager_->findUsersByTag(*tag);
  LOG_INFO("With tag '{}' was finded '{}' users", *tag, list_of_users.size());

  crow::json::wvalue json_users;
  json_users["users"] = crow::json::wvalue::list();

  size_t idx = 0;
  for (const auto& user : list_of_users) {
    auto user_json              = userToJson(user);
    json_users["users"][idx++] = std::move(user_json["user"]);
  }

  return sendResponse( provider_->statusCodes().success, json_users.dump(), false); //todo: check in function code
}

Response AuthController::findById(const RequestDTO& /*req*/, const std::string& user_id_str) {
  std::optional<long long> user_id = getIdFromStr(user_id_str);
  if(!user_id) {
    return sendResponse(provider_->statusCodes().badRequest, "Invalid user_id", true);
  }

  auto found_user = manager_->getUser(*user_id);
  if (!found_user) {
    LOG_ERROR("User with id {} not found", *user_id);
    return sendResponse( provider_->statusCodes().notFound, "User not found", true);
  }

  auto user_json = userToJson(*found_user);
  return sendResponse( provider_->statusCodes().success, user_json["user"].dump(), false);
}

AuthController::OptionalId AuthController::verifyToken(const std::string& token) {
  return authoritizer_->autoritize(token);
}

Response AuthController::registerUser(const RequestDTO& req) {
  auto body = crow::json::load(req.body);
  if (!body || !body.count("email") || !body.count("password") || !body.count("name") || !body.count("tag")) {
    LOG_ERROR("Invalid json");
    return sendResponse(provider_->statusCodes().badRequest, "Invalid json", true);
  }

  RegisterRequest register_request;
  register_request.email    = body["email"].s();
  register_request.password = body["password"].s();
  register_request.name     = body["name"].s();
  register_request.tag      = body["tag"].s();

  LOG_INFO("Registe user (email: {}) | (password : {}) | (name : {}) | (tag: {})",
           register_request.email,
           register_request.password,
           register_request.name,
           register_request.tag);

  //TODO: create middleware for check valid input??
  std::optional<User> register_user = manager_->registerUser(register_request);
  if (!register_user) {
    LOG_ERROR("User not registered");
    return sendResponse(provider_->statusCodes().userError, "User already exist", true);
  }
  LOG_INFO("User registered successfully, id is {}", register_user->id);
  std::string token = generator_->generateToken(register_user->id);
  LOG_INFO("User user {}, id {} token is {}", register_user->username, register_user->id, token);
  return sendResponse(provider_->statusCodes().success, userToJson(*register_user, token).dump(), false);
}
