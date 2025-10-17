#include "controller.h"
#include <QDebug>
#include "headers/TokenService.h"
#include "headers/NetworkManager.h"

using std::string;

Controller::Controller(crow::SimpleApp& app, DataBase& dataBase)
    : app_(app)
    , db(dataBase)
{
    auto initial = db.initialDb();
    if(!initial) {
        LOG_ERROR("DB CHAT isn't initial");
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
        PROFILE_SCOPE("/chats/private");
        auto authHeader = req.get_header_value("Authorization");

        auto myId = TokenService::verifyTokenAndGetUserId(authHeader);
        if(!myId) {
            LOG_ERROR("[CreatePrivateChat] Can't verify token");
            return crow::response(400, "Not valid user token");
        }

        auto body = crow::json::load(req.body);
        if (!body || !body.has("user_id")) {
            LOG_ERROR("[CreatePrivateChat] Missing user_id value");
            return crow::response(400, "Missing userId");
        }


        int userId = body["user_id"].i();
        auto user = NetworkManager::getUserById(userId);

        if(!user) {
            LOG_ERROR("[CreatePrivateChat] User with id '{}' not found", userId);
            return crow::response(405, "User not founded");
        }

        auto chatId = db.createPrivateChat();
        if (!chatId) {
            LOG_ERROR("[CreatePrivateChat] Failed to create chat for user id '{}'", userId);
            return crow::response(500, "Failed to create chat");
        }

        LOG_INFO("[CreatePrivateChat] Created chat with id '{}'", *chatId);

        std::vector members{*myId, userId};
        bool wasAddedMembers = db.addMembersToChat(*chatId, members);
        if(!wasAddedMembers){
            LOG_ERROR("[CreatePrivateChat] Failed to add members in chat '{}'", *chatId);
            db.deleteChat(*chatId);
            return crow::response(502, "Members isn't permitted to add in this chat");
        }

        LOG_INFO("[CreatePrivateChat] Members was added in chat '{}'", *chatId);
        crow::json::wvalue result;
        result["chat_id"] = *chatId;
        result["chat_type"] = "PRIVATE";
        result["title"] = user->name;
        result["avatar"] = user->avatar;
        result["user_id"] = user->id;

        LOG_INFO("Chat: title '{}'", user->name);
        return crow::response(200, result);
    });
}

void Controller::handleGetAllChats(){
    CROW_ROUTE(app_, "/chats").methods(crow::HTTPMethod::GET)
        ([&] (crow::request req) {
        PROFILE_SCOPE("/chats");

        string token = req.get_header_value("Authorization");
        auto userId = TokenService::verifyTokenAndGetUserId(token);
        if (!userId) {
            LOG_ERROR("[GetAllChats] Can't verify token");
            return crow::response(401, "Can't verify token");
        }

        auto chats = db.getChatsOfUser(*userId);
        LOG_INFO("[GetAllChats] Returned chats '{}'", chats.size());
        crow::json::wvalue res;
        res["chats"] = crow::json::wvalue::list();

        int i = 0;
        for (const auto& chat : chats) {
            crow::json::wvalue chatJson;
            chatJson["id"] = chat.id;
            chatJson["type"] = chat.isGroup ? "group" : "private";

            if (chat.isGroup) {
                chatJson["name"] = chat.name;
                chatJson["avatar"] = chat.avatar;
                chatJson["member_count"] = db.getMembersCount(chat.id);
            } else {
                auto otherUserId = db.getOtherMemberId(chat.id, *userId);
                if (otherUserId) {
                    auto user = NetworkManager::getUserById(*otherUserId);
                    chatJson["user"]["id"] = user->id;
                    chatJson["user"]["name"] = user->name;
                    chatJson["user"]["avatar"] = user->avatar;
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
        PROFILE_SCOPE("/chats/<int>");

        string token = req.get_header_value("Authorization");
        if (token.empty()) {
            LOG_ERROR("[GetAllChatsById] Missing token");
            return crow::response(401, "Missing token");
        }

        auto userId = TokenService::verifyTokenAndGetUserId(token);
        if (!userId) {
            LOG_ERROR("[GetAllChatsById] can't verify token");
            return crow::response(401, "Unauthorized");
        }

        auto chatOpt = db.getChatById(chatId);
        if (!chatOpt) {
            LOG_ERROR("[GetAllChatsById] Chat with id '{}' not found", *userId);
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
                    LOG_ERROR("Other user not found for chat '{}'", chatId);
                }
            } else {
                LOG_ERROR("Can't find other members for chat '{}'", chatId);
            }
        }

        return crow::response(200, chatJson);
    });
}

void Controller::handleGetAllChatsMembers(){
    CROW_ROUTE(app_, "/chats/<int>/members").methods(crow::HTTPMethod::GET)
    ([&](int chatId) {
        PROFILE_SCOPE("/chats/<int>/members");

        auto listOfMembers = db.getMembersOfChat(chatId);
        if(!listOfMembers){
            LOG_ERROR("[GetAllChatsMembers] Error in db.getMembersOfChat");
            return crow::response(405, "Error in db.getMembersOfChat");;
        }

        LOG_INFO("Db return '{}' members for chat '{}'", listOfMembers->size(), chatId);
        crow::json::wvalue res;
        res["members"] = crow::json::wvalue::list();

        int i = 0;
        for (auto memberId : *listOfMembers) {
            LOG_INFO("ChatId '{}' - member '{}'", chatId, memberId);
            res["members"][i++] = memberId;
        }

        return crow::response(200, res);
    });
}
