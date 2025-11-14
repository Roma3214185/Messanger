#include "controller.h"

#include "NetworkManager.h"
#include "TokenService.h"
#include "entities/User.h"
#include "NetworkManager.h"
#include "codes.h"
#include "interfaces/IConfigProvider.h"

using std::optional;
using std::string;

namespace {

void sendResponse(crow::response& res, int code, const std::string& text) {
  res.code = code;
  res.write(text);
  res.end();
}

}  // namespace

Controller::Controller(crow::SimpleApp& app, ChatManager* manager,
                       NetworkManager* network_manager, IConfigProvider* provider)
    : app_(app), manager_(manager), network_manager_(network_manager), provider_(provider) {}

void Controller::createPrivateChat(const crow::request& req, crow::response& res) {
  auto auth_header = req.get_header_value("Authorization");

  optional<int> my_id = JwtUtils::verifyTokenAndGetUserId(auth_header);
  if (!my_id) {
    sendResponse(res, provider_->statusCodes().userError, "Invalid or expired token");
    return;
  }

  auto body = crow::json::load(req.body);
  if (!body || !body.has("user_id")) {
    sendResponse(res, provider_->statusCodes().userError, "Missing user_id value");
    return;
  }

  int            user_id = body["user_id"].i();
  optional<User> user    = network_manager_->getUserById(user_id);

  if (!user) {
    sendResponse(res, provider_->statusCodes().userError, "User not found");
    return;
  }

  auto chat_id = manager_->createPrivateChat();
  if (!chat_id) {
    sendResponse(res, provider_->statusCodes().userError, "Failed to create chat");
    return;
  }

  LOG_INFO("[CreatePrivateChat] Created chat with id '{}'", *chat_id);

  std::vector members{*my_id, user_id};
  bool        wasAddedMembers = manager_->addMembersToChat(*chat_id, members);
  if (!wasAddedMembers) {
    sendResponse(res, provider_->statusCodes().serverError, "Failed to add users to chat");
    return;
  }

  crow::json::wvalue result;
  result["chat_id"] = *chat_id;
  result["type"]    = "private";
  result["title"]   = user->username;
  result["avatar"]  = user->avatar;
  result["user_id"] = user->id;

  sendResponse(res, provider_->statusCodes().success, result.dump());
}

void Controller::getAllChats(const crow::request& req, crow::response& res) {
  string token   = req.get_header_value("Authorization");
  auto   user_id = JwtUtils::verifyTokenAndGetUserId(token);
  if (!user_id) {
    sendResponse(res, provider_->statusCodes().userError, "Invalid or expired token");
    return;
  }

  auto chats = manager_->getChatsOfUser(*user_id);
  LOG_INFO("[GetAllChats] Returned chats '{}'", chats.size());

  crow::json::wvalue ans;
  ans["chats"] = crow::json::wvalue::list();

  int i = 0;
  for (const auto& chat : chats) {
    crow::json::wvalue chat_json;
    chat_json["id"]   = chat.id;
    chat_json["type"] = chat.is_group ? "group" : "private";

    if (chat.is_group) {
      chat_json["name"]         = chat.name;
      chat_json["avatar"]       = chat.avatar;
      chat_json["member_count"] = manager_->getMembersCount(chat.id);
    } else {
      auto other_user_id = manager_->getOtherMemberId(chat.id, *user_id);
      if (!other_user_id) {
        LOG_ERROR("I can't get other member id for chat {}", chat.id);
        sendResponse(res, provider_->statusCodes().serverError, "Member not found");
        return;
      }

      auto user = network_manager_->getUserById(*other_user_id);
      if (!user) {
        LOG_ERROR("I can't get user with id '{}'", *other_user_id);
        sendResponse(res, provider_->statusCodes().serverError, "User not found");
        return;
      }

      chat_json["user"]["id"]     = user->id;
      chat_json["user"]["name"]   = user->username;
      chat_json["user"]["avatar"] = user->avatar;
    }

    ans["chats"][i++] = std::move(chat_json);
  }

  sendResponse(res, provider_->statusCodes().success, ans.dump());
}

void Controller::getAllChatsById(const crow::request& req, crow::response& res, int chat_id) {
  string token = req.get_header_value("Authorization");
  if (token.empty()) {
    LOG_ERROR("[GetAllChatsById] Missing token");
    sendResponse(res, provider_->statusCodes().userError, "Missing token");
    return;
  }

  auto user_id = JwtUtils::verifyTokenAndGetUserId(token);
  if (!user_id) {
    LOG_ERROR("[GetAllChatsById] can't verify token");
    sendResponse(res, provider_->statusCodes().userError, "Invalid or expired token");
    return;
  }

  auto chat_opt = manager_->getChatById(chat_id);
  if (!chat_opt) {
    LOG_ERROR("[GetAllChatsById] Chat with id '{}' not found", *user_id);
    sendResponse(res, provider_->statusCodes().userError, "Chat not found");
    return;
  }

  const auto&        chat = *chat_opt;
  crow::json::wvalue chat_json;

  chat_json["id"]   = chat.id;
  chat_json["type"] = chat.is_group ? "group" : "private";

  if (chat.is_group) {
    chat_json["name"]         = chat.name;
    chat_json["avatar"]       = chat.avatar;
    chat_json["member_count"] = manager_->getMembersCount(chat.id);
  } else {  // private
    auto other_user_id = manager_->getOtherMemberId(chat.id, *user_id);
    if (other_user_id) {
      auto user = network_manager_->getUserById(*other_user_id);
      if (user) {
        chat_json["user"]["id"]     = user->id;
        chat_json["user"]["name"]   = user->username;
        chat_json["user"]["avatar"] = user->avatar;
      } else {
        LOG_ERROR("Other user not found for chat '{}'", chat_id);
      }
    } else {
      LOG_ERROR("Can't find other members for chat '{}'", chat_id);
    }
  }

  sendResponse(res, provider_->statusCodes().success, chat_json.dump());
}

void Controller::getAllChatMembers(const crow::request& req, crow::response& res, int chat_id) {
  std::vector<int> list_of_members = manager_->getMembersOfChat(chat_id);
  if (list_of_members.empty()) {
    LOG_ERROR("[GetAllChatsMembers] Error in db.getMembersOfChat");
    sendResponse(res, provider_->statusCodes().serverError, "Error in db.getMembersOfChat");
    return;
  }

  LOG_INFO("Db return '{}' members for chat '{}'", list_of_members.size(), chat_id);
  crow::json::wvalue ans;
  ans["members"] = crow::json::wvalue::list();

  int i = 0;
  for (auto member_id : list_of_members) {
    LOG_INFO("ChatId '{}' - member '{}'", chat_id, member_id);
    ans["members"][i++] = member_id;
  }

  sendResponse(res, provider_->statusCodes().success, ans.dump());
}
