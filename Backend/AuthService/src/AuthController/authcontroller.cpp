#include "authcontroller.h"
#include <QDebug>
#include <iostream>

using std::string;

namespace{

crow::json::wvalue userToJson(const User& user, const std::string& token = "") {
    crow::json::wvalue res;
    if(!token.empty()) res["token"] = token;
    res["user"]["id"] = user.id;
    res["user"]["email"] = user.email;
    res["user"]["name"] = user.name;
    res["user"]["tag"] = user.tag;
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
}

void AuthController::handleLogin(){
    CROW_ROUTE(app_, "/auth/login").methods("POST"_method)(
    [this](const crow::request& req){
        auto body = crow::json::load(req.body);
        if (!body) return crow::response(400, "Invalid JSON");

        string email = body["email"].s();
        string password = body["password"].s();

        auto authRes = service_->loginUser(email, password);
        if (!authRes) return crow::response(401, "Invalid credentials");

        return crow::response(userToJson(*authRes->user, authRes->token));
    });
}


void AuthController::handleMe(){
    CROW_ROUTE(app_, "/auth/me").methods("GET"_method)(
        [this](const crow::request& req){
            auto authRes = verifyToken(req);
            if (!authRes) return crow::response(401, "Invalid or expired token");
            return crow::response(userToJson(*authRes->user, authRes->token));
        });
}

void AuthController::handleRegister(){
    CROW_ROUTE(app_, "/auth/register").methods(crow::HTTPMethod::Post)(
        [this](const crow::request& req){
            auto body = crow::json::load(req.body);
            if (!body) return crow::response(400, "Invalid JSON");

            auto regReq = RegisterRequest{
                .email = body["email"].s(),
                .password = body["password"].s(),
                .name = body["name"].s(),
                .tag = body["tag"].s()
            };

            auto authRes = service_->registerUser(regReq);
            if (!authRes) {
                return crow::response(409, "User already exists");
            }

            std::cout << "[INFO]: I register user (id=" << authRes->user->id << ") " << std::endl;
            return crow::response(userToJson(*authRes->user, authRes->token));
        }
    );
}

void AuthController::handleFindByTag(){
    CROW_ROUTE(app_, "/users/search").methods(crow::HTTPMethod::GET)(
        [this](const crow::request& req) {
            auto tag = req.url_params.get("tag");
            if (!tag) {
                return crow::response(400, "Missing 'tag' parameter");
            }

            auto listOfUsers = service_->findUserByTag(tag);
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
            auto foundUser = service_->findUserById(userId);
            if(!foundUser) {
                return crow::response{404, "Users not found"};
            }

            std::cout << "[INFO] User found with id: " << userId << std::endl;
            auto userJson = userToJson(*foundUser);
            return crow::response(200, userJson["user"]);
        }
    );
}

std::optional<AuthResponce> AuthController::verifyToken(const crow::request& req) {
    auto token = req.get_header_value("Authorization");
    if(token.empty()) {
        qDebug() << "Token empty";
        return std::nullopt;
    }
    return service_->getUser(token);
}
