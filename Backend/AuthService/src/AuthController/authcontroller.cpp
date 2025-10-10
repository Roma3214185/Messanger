#include "authcontroller.h"
#include <QDebug>
#include <iostream>


AuthController::AuthController(crow::SimpleApp& app, AuthManager* service)
    : app_(app)
    , service_(service){ }

void AuthController::initRoutes(){
    std::cout << "I INIT ROUTES" << std::endl;
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


        auto email = body["email"].s();
        auto password = body["password"].s();


        auto authRes = service_->loginUser(email, password);
        if (!authRes) return crow::response(401, "Invalid credentials");

        crow::json::wvalue res;
        res["token"] = authRes->token;
        res["user"]["id"] = authRes->user->id;
        res["user"]["email"] = authRes->user->email;
        res["user"]["name"] = authRes->user->name;
        res["user"]["tag"] = authRes->user->tag;

        return crow::response(res);
    });
}


void AuthController::handleMe(){
    CROW_ROUTE(app_, "/auth/me").methods("GET"_method)(
        [this](const crow::request& req){
            auto token = req.get_header_value("Authorization");
            if (token.empty()) {
                qDebug() << "[ERROR] Token is empty";
                return crow::response(401, "Missing token");
            }

            auto authRes = service_->getUser(token);
            if (!authRes) {
                qDebug() << "[ERROR] Invalid or expired token";
                return crow::response(401, "Invalid or expired token");
            }

            crow::json::wvalue res;
            res["token"] = authRes->token;
            res["user"]["id"] = authRes->user->id;
            res["user"]["email"] = authRes->user->email;
            res["user"]["name"] = authRes->user->name;
            res["user"]["tag"] = authRes->user->tag;

            return crow::response(res);
        });
}

void AuthController::handleRegister(){
    CROW_ROUTE(app_, "/auth/register").methods(crow::HTTPMethod::Post)(
        [this](const crow::request& req){
        std::cout << "[register]: start" << std::endl;
            auto body = crow::json::load(req.body);
            if (!body) return crow::response(400, "Invalid JSON");

            RegisterRequest regReq{
                .email = body["email"].s(),
                .password = body["password"].s(),
                .name = body["name"].s(),
                .tag = body["tag"].s()
            };
            std::cout << "[register]: request to service" << regReq.email << std::endl;

            auto authRes = service_->registerUser(regReq);
            if (!authRes) return crow::response(409, "User already exists");
            std::cout << "[register]: finish successfull" << std::endl;


            crow::json::wvalue res;
            res["token"] = authRes->token;
            res["user"]["id"] = authRes->user->id;
            res["user"]["email"] = authRes->user->email;
            res["user"]["name"] = authRes->user->name;
            res["user"]["tag"] = authRes->user->tag;
            std::cout << "[register]: finish" << authRes->user->email << std::endl;

            return crow::response(res);
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

            std::cout << "[search]: finding by tag = " << tag << std::endl;

            auto findedUsers = service_->findUserByTag(tag); // QList<User>

            crow::json::wvalue res;
            res["users"] = crow::json::wvalue::list();

            size_t i = 0;
            for (const auto& user : findedUsers) {
                crow::json::wvalue u;
                u["id"] = user.id;
                u["email"] = user.email;
                u["name"] = user.name;
                u["tag"] = user.tag;
                res["users"][i++] = std::move(u);
            }

            // Повертаємо навіть якщо список порожній
            return crow::response(200, res);
        }
        );
}

void AuthController::handleFindById(){
    CROW_ROUTE(app_, "/users/<int>").methods(crow::HTTPMethod::GET)(
        [this](const crow::request& req, int userId) {


            std::cout << "[search]: finding by id = " << userId << std::endl;

            auto findedUsers = service_->findUserById(userId);
            if(!findedUsers) {
                std::cout << "[ERROR] User not found with id: " << userId << std::endl;
                return crow::response{405, "Users with id not fount"};
            }

            std::cout << "[INFO] User found with id: " << userId << std::endl;

            crow::json::wvalue res;

                res["id"] = findedUsers->id;
                res["email"] = findedUsers->email;
                res["name"] = findedUsers->name;
                res["tag"] = findedUsers->tag;


            return crow::response(200, res);
        }
    );
}
