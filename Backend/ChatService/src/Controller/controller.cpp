#include "controller.h"

#include "headers/NetworkManager.h"
#include "headers/TokenService.h"

using std::optional;
using std::string;

constexpr int kUserError = 400;
constexpr int kServerError = 500;
constexpr int kSuccessfullCode = 200;

Controller::Controller(crow::SimpleApp& app, ChatManager* manager)
    : app_(app), manager_(manager) {

}

void Controller::handleRoutes() {
  handleCreatingPrivateChat();
  handleGetAllChats();
  handleGetAllChatsById();
  handleGetAllChatsMembers();
}

void Controller::handleCreatingPrivateChat() {
  CROW_ROUTE(app_, "/chats/private")
      .methods(
          crow::HTTPMethod::POST)  // TODO(roma): Add private chat with user
      ([&](crow::request req) {
        PROFILE_SCOPE("/chats/private");
        auto auth_header = req.get_header_value("Authorization");

        optional<int> my_id =
            JwtUtils::verifyTokenAndGetUserId(auth_header);
        if (!my_id) {
          LOG_ERROR("[CreatePrivateChat] Can't verify token");
          return crow::response(kUserError, "Not valid user token");
        }

        auto body = crow::json::load(req.body);
        if (!body || !body.has("user_id")) {
          LOG_ERROR("[CreatePrivateChat] Missing user_id value");
          return crow::response(kUserError, "Missing userId");
        }

        int user_id = body["user_id"].i();
        optional<User> user = NetworkManager::getUserById(user_id);

        if (!user) {
          LOG_ERROR("[CreatePrivateChat] User with id '{}' not found", user_id);
          return crow::response(kUserError, "User not founded");
        }

        auto chat_id = manager_->createPrivateChat();
        if (!chat_id) {
          LOG_ERROR(
              "[CreatePrivateChat] Failed to create chat for user id '{}'",
              user_id);
          return crow::response(kServerError, "Failed to create chat");
        }

        LOG_INFO("[CreatePrivateChat] Created chat with id '{}'", *chat_id);

        std::vector members{*my_id, user_id};
        bool wasAddedMembers = manager_->addMembersToChat(*chat_id, members);
        if (!wasAddedMembers) {
          LOG_ERROR("[CreatePrivateChat] Failed to add members in chat '{}'",
                    *chat_id);
          //manager_->deleteChat(*chat_id);
          return crow::response(kUserError,
                                "Members isn't permitted to add in this chat");
        }

        LOG_INFO("[CreatePrivateChat] Members was added in chat '{}'",
                 *chat_id);
        crow::json::wvalue result;
        result["chat_id"] = *chat_id;
        result["type"] = "private";
        result["title"] = user->name;
        result["avatar"] = user->avatar;
        result["user_id"] = user->id;

        LOG_INFO("Chat: title '{}'", user->name);
        return crow::response(kSuccessfullCode, result);
      });
}

void Controller::handleGetAllChats() {
  CROW_ROUTE(app_, "/chats")
      .methods(crow::HTTPMethod::GET)([&](crow::request req) -> crow::response {
        PROFILE_SCOPE("/chats");

        string token = req.get_header_value("Authorization");
        auto user_id = JwtUtils::verifyTokenAndGetUserId(token);
        if (!user_id) {
          LOG_ERROR("[GetAllChats] Can't verify token");
          return crow::response(kUserError, "Can't verify token");
        }

        auto chats = manager_->getChatsOfUser(*user_id);
        LOG_INFO("[GetAllChats] Returned chats '{}'", chats.size());
        crow::json::wvalue res;
        res["chats"] = crow::json::wvalue::list();

        int i = 0;
        for (const auto& chat : chats) {
          crow::json::wvalue chat_json;
          chat_json["id"] = chat.id;
          chat_json["type"] = chat.is_group ? "group" : "private";

          if (chat.is_group) {
            chat_json["name"] = chat.name;
            chat_json["avatar"] = chat.avatar;
            chat_json["member_count"] = manager_->getMembersCount(chat.id);
          } else {
            auto other_user_id = manager_->getOtherMemberId(chat.id, *user_id);
            if (!other_user_id) {
              LOG_ERROR("I can't get other member id");
              return crow::response(kServerError);
            }

            auto user = NetworkManager::getUserById(*other_user_id);
            if (!user) {
              LOG_ERROR("I can't get user with id '{}'", *other_user_id);
              return crow::response(kServerError);
            }

            chat_json["user"]["id"] = user->id;
            chat_json["user"]["name"] = user->name;
            chat_json["user"]["avatar"] = user->avatar;
          }

          res["chats"][i++] = std::move(chat_json);
        }

        return crow::response(kSuccessfullCode, res);
      });
}

void Controller::handleGetAllChatsById() {
  CROW_ROUTE(app_, "/chats/<int>")
      .methods(
          crow::HTTPMethod::GET)([&](const crow::request& req, int chat_id) {
        PROFILE_SCOPE("/chats/<int>");

        string token = req.get_header_value("Authorization");
        if (token.empty()) {
          LOG_ERROR("[GetAllChatsById] Missing token");
          return crow::response(kUserError, "Missing token");
        }

        auto user_id = JwtUtils::verifyTokenAndGetUserId(token);
        if (!user_id) {
          LOG_ERROR("[GetAllChatsById] can't verify token");
          return crow::response(kUserError, "Unauthorized");
        }

        auto chat_opt = manager_->getChatById(chat_id);
        if (!chat_opt) {
          LOG_ERROR("[GetAllChatsById] Chat with id '{}' not found", *user_id);
          return crow::response(kUserError, "Chat not found");
        }

        const auto& chat = *chat_opt;
        crow::json::wvalue chat_json;

        chat_json["id"] = chat.id;
        chat_json["type"] = chat.is_group ? "group" : "private";

        if (chat.is_group) {
          chat_json["name"] = chat.name;
          chat_json["avatar"] = chat.avatar;
          chat_json["member_count"] = manager_->getMembersCount(chat.id);

        } else {  // private
          auto other_user_id = manager_->getOtherMemberId(chat.id, *user_id);
          if (other_user_id) {
            auto user = NetworkManager::getUserById(*other_user_id);
            if (user) {
              chat_json["user"]["id"] = user->id;
              chat_json["user"]["name"] = user->name;
              chat_json["user"]["avatar"] = user->avatar;
            } else {
              LOG_ERROR("Other user not found for chat '{}'", chat_id);
            }
          } else {
            LOG_ERROR("Can't find other members for chat '{}'", chat_id);
          }
        }

        return crow::response(kSuccessfullCode, chat_json);
      });
}

void Controller::handleGetAllChatsMembers() {
  CROW_ROUTE(app_, "/chats/<int>/members")
      .methods(crow::HTTPMethod::GET)([&](int chat_id) {
        PROFILE_SCOPE("/chats/<int>/members");

        std::vector<int> list_of_members =
            manager_->getMembersOfChat(chat_id);
        if (list_of_members.empty()) {
          LOG_ERROR("[GetAllChatsMembers] Error in db.getMembersOfChat");
          return crow::response(kServerError, "Error in db.getMembersOfChat");
        }

        LOG_INFO("Db return '{}' members for chat '{}'",
                 list_of_members.size(), chat_id);
        crow::json::wvalue res;
        res["members"] = crow::json::wvalue::list();

        int i = 0;
        for (auto member_id : list_of_members) {
          LOG_INFO("ChatId '{}' - member '{}'", chat_id, member_id);
          res["members"][i++] = member_id;
        }

        return crow::response(kSuccessfullCode, res);
      });
}
