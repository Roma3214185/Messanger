#include "authcontroller.h"

#include <string>
#include <utility>

#include "Debug_profiling.h"
#include "JwtUtils.h"
#include "entities/RegisterRequest.h"

using std::string;

constexpr int kUserError      = 400;
constexpr int kSuccessfulCode = 200;

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

}  // namespace

AuthController::AuthController(crow::SimpleApp& app, AuthManager* service)
    : app_(app), service_(service) {}

void AuthController::loginUser(const crow::request& req, crow::response& responce) {
  auto body = crow::json::load(req.body);
  if (!body) {
    sendResponse(responce, kUserError, "Invalid Json");
    return;
  }
  LoginRequest login_request{
      .email    = body["email"].s(),
      .password = body["password"].s(),
  };

  LOG_INFO("[login] user email: '{}' and user password '{}'",
           login_request.email,
           login_request.password);

  auto auth_res = service_->loginUser(login_request);

  if (!auth_res) {
    sendResponse(responce, kUserError, "Invalid credentials");
  } else {
    sendResponse(responce, kSuccessfulCode, userToJson(*auth_res->user, auth_res->token).dump());
  }
}

void AuthController::handleMe(const crow::request& req, crow::response& responce) {
  auto authRes = verifyToken(req);
  if (!authRes) {
    sendResponse(responce, kUserError, "Invalid or expired token");
  } else {
    sendResponse(responce, kSuccessfulCode, userToJson(*authRes->user, authRes->token).dump());
  }
}

void AuthController::findByTag(const crow::request& req,
                               const std::string&   tag,
                               crow::response&      responce) {
  if (tag.empty()) {
    sendResponse(responce, kUserError, "Missing tag parametr");
    return;
  }

  auto listOfUsers = service_->findUserByTag(tag);
  LOG_INFO("With tag '{}' was finded '{}' users", tag, listOfUsers.size());

  crow::json::wvalue json_users;
  json_users["users"] = crow::json::wvalue::list();

  size_t idx = 0;
  for (const auto& user : listOfUsers) {
    auto userJson              = userToJson(user);
    json_users["users"][idx++] = std::move(userJson["user"]);
  }

  sendResponse(responce, kSuccessfulCode, json_users.dump());
}

void AuthController::findById(const crow::request& req, int user_id, crow::response& responce) {
  auto found_user = service_->findUserById(user_id);
  if (!found_user) {
    sendResponse(responce, kUserError, "Users not found");
  } else {
    auto user_json = userToJson(*found_user);
    sendResponse(responce, kSuccessfulCode, user_json["user"].dump());
  }
}

std::optional<AuthResponce> AuthController::verifyToken(const crow::request& req) {
  auto token = req.get_header_value("Authorization");
  if (token.empty()) {
    LOG_ERROR("Token empty");
    return std::nullopt;
  }

  return service_->getUser(token);
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
  if (!body) {
    sendResponse(responce, kUserError, "Invalid json");
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

  auto auth_responce = service_->registerUser(register_request);
  if (!auth_responce) {
    sendResponse(responce, kUserError, "User already exist");
    return;
  } else {
    sendResponse(
        responce, kSuccessfulCode, userToJson(*auth_responce->user, auth_responce->token).dump());
  }
}
