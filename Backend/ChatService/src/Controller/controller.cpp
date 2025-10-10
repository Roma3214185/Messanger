#include "controller.h"
#include <QDebug>
#include "headers/TokenService.h"
#include "headers/NetworkManager.h"

Controller::Controller(crow::SimpleApp& app, DataBase& dataBase)
    : app_(app)
    , db(dataBase)
{
    auto initial = db.initialDb();
    if(!initial) {
        qDebug() << "[ERROR] bd isn't initial";
        throw std::runtime_error("BD not initial");
    }
}

void Controller::handleRoutes(){
    handleCreatingPrivateChat();
    handleGetAllChats();
    handleGetAllChatsById();
    handleGetAllChatsMembers();
}

void Controller::handleCreatingPrivateChat(){
CROW_ROUTE(app_, "/chats/private").methods(crow::HTTPMethod::POST) /// Add private chat with user
    ([&](crow::request req){
        auto authHeader = req.get_header_value("Authorization");

        auto myId = TokenService::verifyTokenAndGetUserId(authHeader);
        if(!myId) {
            qDebug() << "[ERROR] Invelid myId";
            return crow::response(400, "Not valid user token");
        }

        auto body = crow::json::load(req.body);
        if (!body || !body.has("user_id")) {
            qDebug() << "[ERROR] Invelid userId";
            return crow::response(400, "Missing userId");
        }


        int userId = body["user_id"].i();

        auto user = NetworkManager::getUserById(userId);
        if(!user) return crow::response(405, "User not founded");
        qDebug() << "[INFO] User to create chat with finded " << userId << *myId;


        //check if u can to createChatWIthThisUser, or db can check for himself??
        std::optional<int> chatId = db.createPrivateChat();

        if (!chatId) {
            return crow::response(500, "Failed to create chat");
        }
        qDebug() << "[INFO] created chat with id" << *chatId;

        std::vector members{*myId, userId};
        bool wasAddedMembers = db.addMembersToChat(*chatId, members);
        if(!wasAddedMembers){
            db.deleteChat(*chatId);
            return crow::response(502, "Members isn't permitted to add in this chat");
        }

        crow::json::wvalue result;
        result["chat_id"] = *chatId;
        result["chat_type"] = "PRIVATE";
        result["title"] = user->name;
        result["avatar"] = user->avatar;
        result["user_id"] = user->id;

        return crow::response(200, result);
    });
}

void Controller::handleGetAllChats(){
CROW_ROUTE(app_, "/chats").methods(crow::HTTPMethod::GET)
    ([&] (crow::request req) {
        qDebug() << "[INFO] get all chats";
        std::string token = req.get_header_value("Authorization");
        qDebug() << "[INFO] token = " << token;
        auto userId = TokenService::verifyTokenAndGetUserId(token);
        if (!userId) {
            qDebug() << "[ERRPR] can't verify token";
            return crow::response(401, "Unauthorized");
        }

        qDebug() << "[INFO] send request to db";
        auto chats = db.getChatsOfUser(*userId);
        qDebug() << "[INFO] returned chats size" << chats.size();
        crow::json::wvalue res;
        res["chats"] = crow::json::wvalue::list();

        int i = 0;
        for (const auto& chat : chats) {
            crow::json::wvalue chatJson;
            chatJson["id"] = chat.id;
            chatJson["type"] = chat.isGroup ? "group" : "private";

            if (chat.isGroup) {
                // 🟢 Група
                chatJson["name"] = chat.name;
                chatJson["avatar"] = chat.avatar;
                chatJson["member_count"] = db.getMembersCount(chat.id);
            } else {
                // 🔵 Приватний чат → знайти іншого користувача
                auto otherUserId = db.getOtherMemberId(chat.id, *userId);
                if (otherUserId) {
                    auto user = NetworkManager::getUserById(*otherUserId);
                    chatJson["user"]["id"] = user->id;
                    chatJson["user"]["name"] = user->name;
                    chatJson["user"]["avatar"] = user->avatar; //later u will be have this a bug
                }
            }

            res["chats"][i++] = std::move(chatJson);
        }

        return crow::response(200, res);
    });
}

void Controller::handleGetAllChatsById(){
CROW_ROUTE(app_, "/chats/<int>").methods(crow::HTTPMethod::GET)
    ([&](const crow::request& req, int chatId) {
        qDebug() << "[INFO] get single chat by id =" << chatId;

        std::string token = req.get_header_value("Authorization");
        if (token.empty()) {
            qDebug() << "[ERROR] Missing Authorization header";
            return crow::response(401, "Missing token");
        }

        auto userId = TokenService::verifyTokenAndGetUserId(token);
        if (!userId) {
            qDebug() << "[ERROR] Invalid or expired token";
            return crow::response(401, "Unauthorized");
        }

        // --- Отримати чат з бази ---
        auto chatOpt = db.getChatById(chatId);
        if (!chatOpt) {
            qDebug() << "[WARN] Chat not found:" << chatId;
            return crow::response(404, "Chat not found");
        }

        const auto& chat = *chatOpt;
        crow::json::wvalue chatJson;

        chatJson["id"] = chat.id;
        chatJson["type"] = chat.isGroup ? "group" : "private";

        if (chat.isGroup) {
            chatJson["name"] = chat.name;
            chatJson["avatar"] = chat.avatar;
            chatJson["member_count"] = db.getMembersCount(chat.id);

        } else { //private
            auto otherUserId = db.getOtherMemberId(chat.id, *userId);
            if (otherUserId) {
                auto user = NetworkManager::getUserById(*otherUserId);
                if (user) {
                    chatJson["user"]["id"] = user->id;
                    chatJson["user"]["name"] = user->name;
                    chatJson["user"]["avatar"] = user->avatar;
                } else {
                    qDebug() << "[ERROR] other user not found for chat" << chatId;
                }
            } else {
                qDebug() << "[ERROR] can't find other member for chat" << chatId;
            }
        }

        qDebug() << "[INFO] Returning chat JSON for" << chatId;
        return crow::response(200, chatJson);
    });
}

void Controller::handleGetAllChatsMembers(){
CROW_ROUTE(app_, "/chats/<int>/members").methods(crow::HTTPMethod::GET)
    ([&](int chatId) {
        qDebug() << "[INFO] getMembersOfChat " << chatId << " id";
        auto listOfMembers = db.getMembersOfChat(chatId);
        if(!listOfMembers){
            qDebug() << "[ERROR] getMembersOfChat with id: " << chatId << " id";
            return crow::response(405, "ERROR IN GETTING MEMBERS");;
        }
        qDebug() << "[INFO] return chatmembers " << listOfMembers->size();

        // Створюємо JSON-масив
        crow::json::wvalue res;
        res["members"] = crow::json::wvalue::list();

        int i = 0;
        for (auto memberId : *listOfMembers) {
            res["members"][i++] = memberId;
        }

        return crow::response(200, res);
    });
}
