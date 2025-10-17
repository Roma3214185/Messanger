#include "authcontroller.h"
#include <QDebug>
#include <iostream>
#include "../../../DebugProfiling/Debug_profiling.h"

using std::string;

namespace{

crow::json::wvalue userToJson(const User& user, const std::string& token = "") {
    crow::json::wvalue res;
    if(!token.empty()) res["token"] = token;
    res["user"]["id"] = user.id;
    res["user"]["email"] = user.email;
    res["user"]["name"] = user.username;
    res["user"]["tag"] = user.tag;

    LOG_INFO("[user][id] = '{}' | [email] = '{}' | "
                 "[name] = '{}' | [tag] = '{}', token = '{}'", user.id, user.username, user.email, user.tag, token);
    return res;
}

} //namespace

AuthController::AuthController(crow::SimpleApp& app, AuthManager* service)
    : app_(app)
    , service_(service)
{

}

void AuthController::initRoutes(){
    handleLogin();
    handleRegister();
    handleMe();
    handleFindByTag();
    handleFindById();
    spdlog::debug("[authcontroller] routes inited");
}

void AuthController::handleLogin(){
    CROW_ROUTE(app_, "/auth/login").methods("POST"_method)(
    [this](const crow::request& req){
        PROFILE_SCOPE("/auth/login");
        auto body = crow::json::load(req.body);
        if (!body) {
            LOG_ERROR("[Login] Invalid Json");
            return crow::response(400, "Invalid JSON");
        }

        string email = body["email"].s();
        string password = body["password"].s();
        LOG_INFO("[login] user email: '{}' and user password '{}'", email, password);

        auto authRes = service_->loginUser(email, password);

        if (!authRes) {
            LOG_ERROR("[login] invalid credentials");
            return crow::response(401, "Invalid credentials");
        }
        LOG_ERROR("[login] successfull: name '{}' | id '{}' | tag '{}'", (*authRes->user).username, (*authRes->user).id, (*authRes->user).tag);
        return crow::response(userToJson(*authRes->user, authRes->token));
    });
}


void AuthController::handleMe(){
    CROW_ROUTE(app_, "/auth/me").methods("GET"_method)(
        [this](const crow::request& req){
            PROFILE_SCOPE("/auth/me");
            auto authRes = verifyToken(req);
            if (!authRes) {
                LOG_ERROR("Ivalid or expired token");
                return crow::response(401, "Invalid or expired token");
            }
            return crow::response(userToJson(*authRes->user, authRes->token));
        });
}

void AuthController::handleRegister(){
    CROW_ROUTE(app_, "/auth/register").methods(crow::HTTPMethod::Post)(
        [this](const crow::request& req){
            PROFILE_SCOPE("/auth/register");
            auto body = crow::json::load(req.body);
            if (!body) {
                LOG_ERROR("[register] invalid json");
                return crow::response(400, "Invalid JSON");
            }

            auto regReq = RegisterRequest{
                .email = body["email"].s(),
                .password = body["password"].s(),
                .name = body["name"].s(),
                .tag = body["tag"].s()
            };

            auto authRes = service_->registerUser(regReq);
            if (!authRes) {
                LOG_ERROR("[register] User already exists");
                return crow::response(409, "User already exists");
            }

            LOG_INFO("[register] user (id='{}' ", authRes->user->id);
            return crow::response(userToJson(*authRes->user, authRes->token));
        }
    );
}

void AuthController::handleFindByTag(){
    CROW_ROUTE(app_, "/users/search").methods(crow::HTTPMethod::GET)(
        [this](const crow::request& req) {
            PROFILE_SCOPE("[/users/search]");
            auto tag = req.url_params.get("tag");
            if (!tag) {
                LOG_ERROR("Missing tag parametr");
                return crow::response(400, "Missing 'tag' parameter");
            }

            auto listOfUsers = service_->findUserByTag(tag);
            LOG_INFO("With tag '{}' was finded '{}' users", tag, listOfUsers.size());

            crow::json::wvalue res;
            res["users"] = crow::json::wvalue::list();

            size_t i = 0;
            for (const auto& user : listOfUsers) {
                auto userJson = userToJson(user);
                res["users"][i++] = std::move(userJson["user"]);
            }

            return crow::response(200, res);
        }
    );
}

void AuthController::handleFindById(){
    CROW_ROUTE(app_, "/users/<int>").methods(crow::HTTPMethod::GET)(
        [this](const crow::request& req, int userId) {
            PROFILE_SCOPE("/users/id");
            auto foundUser = service_->findUserById(userId);
            if(!foundUser) {
                LOG_ERROR("[handleById] User not found with id '{}'", userId);
                return crow::response{404, "Users not found"};
            }

            LOG_INFO("[handleById] User found with id '{}'", userId);
            auto userJson = userToJson(*foundUser);
            return crow::response(200, userJson["user"]);
        }
    );
}

std::optional<AuthResponce> AuthController::verifyToken(const crow::request& req) {
    auto token = req.get_header_value("Authorization");
    if(token.empty()) {
        LOG_ERROR("Token empty");
        return std::nullopt;
    }

    return service_->getUser(token);
}
