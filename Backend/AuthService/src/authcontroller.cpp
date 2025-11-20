#include "authservice/authcontroller.h"

#include <string>
#include <utility>

#include "Debug_profiling.h"
#include "authservice/JwtUtils.h"
#include "entities/RegisterRequest.h"
#include "authservice/interfaces/IAuthManager.h"
#include "entities/AuthResponce.h"
#include "interfaces/IAutoritizer.h"

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

  LOG_INFO(
      "[user][id] = '{}' | [email] = '{}' | "
      "[name] = '{}' | [tag] = '{}', token = '{}'",
      user.id,
      user.username,
      user.email,
      user.tag,
      token);
  return res;
}

void sendResponse(crow::response& res, int code, const std::string& text) {
  res.code = code;
  res.write(text);
  res.end();
}

void saveInFile(const std::string& file_name, const std::string& key) {
  std::ofstream file(file_name);
  if (!file) throw std::runtime_error("Cannot open file for writing");
  file << key;
  file.close();
}

std::string fetchTag(const crow::request& req) {
  const char* t = req.url_params.get("tag");
  if (!t) {
    // tag does not exist
    return "";
  }
  return std::string(t);
}

}  // namespace

AuthController::AuthController(IAuthManager* manager, IAutoritizer* authoritizer, IConfigProvider* provider)
    : manager_(manager), authoritizer_(authoritizer), provider_(provider) {}

void AuthController::loginUser(const crow::request& req, crow::response& responce) {
  auto body = crow::json::load(req.body);
  if (!body || !body.count("email") || !body.count("password")) {
    sendResponse(responce, provider_->statusCodes().badRequest, "Invalid Json");
    return;
  }
  LoginRequest login_request{
      .email    = body["email"].s(),
      .password = body["password"].s(),
  };

  LOG_INFO("[login] user email: '{}' and user password '{}'",
           login_request.email,
           login_request.password);

  std::optional<User> logged_user = manager_->loginUser(login_request);

  if (!logged_user) {
    return sendResponse(responce, provider_->statusCodes().unauthorized, "Invalid credentials");
  }

  auto token = generateToken(logged_user->id);
  sendResponse(responce, provider_->statusCodes().success, userToJson(*logged_user, token).dump());
}

void AuthController::handleMe(const crow::request& req, crow::response& responce) {
  auto [user_id, token] = verifyToken(req);
  if (!user_id) {
    return sendResponse(responce, provider_->statusCodes().unauthorized, provider_->statusCodes().invalidToken);
  }

  std::optional<User> user = manager_->getUser(*user_id);
  if(!user) {
    return sendResponse(responce, provider_->statusCodes().notFound, "User not found");
  }

  sendResponse(responce, provider_->statusCodes().success, userToJson(*user, token).dump());
}

void AuthController::findByTag(const crow::request& req,
                               crow::response&      responce) {
  std::string tag = fetchTag(req);
  if (tag.empty()) {
    return sendResponse(responce, provider_->statusCodes().badRequest, "Missing tag parametr");
  }

  auto listOfUsers = manager_->findUserByTag(tag);
  LOG_INFO("With tag '{}' was finded '{}' users", tag, listOfUsers.size());

  crow::json::wvalue json_users;
  json_users["users"] = crow::json::wvalue::list();

  size_t idx = 0;
  for (const auto& user : listOfUsers) {
    auto userJson              = userToJson(user);
    json_users["users"][idx++] = std::move(userJson["user"]);
  }

  sendResponse(responce, provider_->statusCodes().success, json_users.dump());
}

void AuthController::findById(const crow::request& req, int user_id, crow::response& responce) {
  auto found_user = manager_->getUser(user_id);
  if (!found_user) {
    return sendResponse(responce, provider_->statusCodes().notFound, "User not found");
  }

  auto user_json = userToJson(*found_user);
  sendResponse(responce, provider_->statusCodes().success, user_json["user"].dump());
}

std::pair<std::optional<long long>, std::string> AuthController::verifyToken(const crow::request& req) {
  auto token = req.get_header_value("Authorization");
  return std::make_pair(authoritizer_->autoritize(token), token);

  // auto user = manager_->getUser(*id);
  // if(!user) return std::nullopt;

  // return AuthResponce{.token = token, .user = user};
}

std::string AuthController::generateToken(int user_id) {
  return JwtUtils::generateToken(user_id);
}

void AuthController::generateKeys() {
  const std::string kKeysDir        = "/Users/roma/QtProjects/Chat/Backend/shared_keys/";
  const std::string kPrivateKeyFile = "private_key.pem";
  const std::string kPublicKeyFile  = kKeysDir + "public_key.pem";

  auto [private_key, public_key] = JwtUtils::generate_rsa_keys();
  std::filesystem::create_directories(kKeysDir);

  try {
    saveInFile(kPrivateKeyFile, private_key);
    saveInFile(kPublicKeyFile, public_key);

    LOG_INFO("Save private_key in {}: {}", kPrivateKeyFile, private_key);
    LOG_INFO("Save public_key in {}: {}", kPublicKeyFile, public_key);
  } catch (...) {
    LOG_ERROR("Error saving keys");
  }
}

void AuthController::registerUser(const crow::request& req, crow::response& responce) {
  auto body = crow::json::load(req.body);
  if (!body || !body.count("email") || !body.count("password") || !body.count("name") || !body.count("tag")) {
    sendResponse(responce, provider_->statusCodes().badRequest, "Invalid json");
    return;
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

  std::optional<User> register_user = manager_->registerUser(register_request);
  if (!register_user) {
    return sendResponse(responce, provider_->statusCodes().userError, "User already exist");
  }

  std::string token = generateToken(register_user->id);
  sendResponse(responce, provider_->statusCodes().success, userToJson(*register_user, token).dump());
}
