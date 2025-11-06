#include "authcontroller.h"

#include <string>
#include <utility>

#include "Debug_profiling.h"
#include "RegisterRequest.h"
#include "JwtUtils.h"

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

void AuthController::loginUser(const crow::request& req, crow::response& responce) {
  auto body = crow::json::load(req.body);
  if (!body) {
    responce.code = kUserError;
    responce.write("Invalid Json");
    responce.end();
    return;
  }
  LoginRequest login_request{
      .email = body["email"].s(),
      .password = body["password"].s(),
  };

  LOG_INFO("[login] user email: '{}' and user password '{}'",
           login_request.email, login_request.password);

  auto auth_res = service_->loginUser(login_request);

  if (!auth_res) {
    responce.code = kUserError;
    responce.write("Invalid credentials");
  } else {
    responce.code = kSuccessfulCode;
    responce.write(userToJson(*auth_res->user, auth_res->token).dump());
  }
  responce.end();
}

void AuthController::handleMe(const crow::request& req, crow::response& responce) {
  auto authRes = verifyToken(req);
  if (!authRes) {
    responce.code = kUserError;
    responce.write("Invalid or expired token");
  } else {
    responce.code = kSuccessfulCode;
    responce.write(userToJson(*authRes->user, authRes->token).dump());
  }
  responce.end();
}

void AuthController::findByTag(const crow::request& req, const std::string& tag, crow::response& responce) {
  if (tag.empty()) {
    responce.code = kUserError;
    responce.write("Missing tag parametr");
    responce.end();
    return;
  }

  auto listOfUsers = service_->findUserByTag(tag);
  LOG_INFO("With tag '{}' was finded '{}' users", tag,
           listOfUsers.size());

  crow::json::wvalue json_users;
  json_users["users"] = crow::json::wvalue::list();

  size_t idx = 0;
  for (const auto& user : listOfUsers) {
    auto userJson = userToJson(user);
    json_users["users"][idx++] = std::move(userJson["user"]);
  }

  responce.code = kSuccessfulCode;
  responce.write(json_users.dump());
  responce.end();
}

void AuthController::findById(const crow::request& req, int user_id, crow::response& responce) {
  auto found_user = service_->findUserById(user_id);
  if (!found_user) {
    responce.code = kUserError;
    responce.write("Users not found");
  } else {
    auto user_json = userToJson(*found_user);
    responce.code = kSuccessfulCode;
    responce.write(user_json["user"].dump());
  }
  responce.end();
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

void AuthController::generateKeys() {
  const std::string kKeysDir = "/Users/roma/QtProjects/Chat/Backend/shared_keys/";
  const std::string kPrivateKeyFile = "private_key.pem";
  const std::string kPublicKeyFile = kKeysDir + "public_key.pem";

  auto [private_key, public_key] = JwtUtils::generate_rsa_keys();
  std::filesystem::create_directories(kKeysDir);
  std::ofstream privFile(kPrivateKeyFile);
  if (!privFile) throw std::runtime_error("Cannot open private_key.pem for writing");
  privFile << private_key;
  privFile.close();

  std::ofstream publFile(kPublicKeyFile);
  if (!publFile) throw std::runtime_error("Cannot open public_key.pem for writing");
  publFile << public_key;
  publFile.close();

  LOG_INFO("Save private_key in {}: {}", kPrivateKeyFile, private_key);
  LOG_INFO("Save public_key in {}: {}", kPublicKeyFile, public_key);
}

void AuthController::registerUser(const crow::request& req, crow::response& responce) {
  auto body = crow::json::load(req.body);
  if (!body) {
    responce.code = kUserError;
    responce.write("Invalid json");
    responce.end();
    return;
  }

  RegisterRequest register_request;
  register_request.email = body["email"].s();
  register_request.password = body["password"].s();
  register_request.name = body["name"].s();
  register_request.tag = body["tag"].s();

  LOG_INFO("Registe user (email: {}) | (password : {}) | (name : {}) | (tag: {})",
           register_request.email,
           register_request.password,
           register_request.name,
           register_request.tag);

  auto auth_responce = service_->registerUser(register_request);
  if (!auth_responce) {
    responce.code = kUserError;
    responce.write("User already exist");
  } else {
    responce.code = kSuccessfulCode;
    responce.write(userToJson(*auth_responce->user, auth_responce->token).dump());
  }
  responce.end();
}
