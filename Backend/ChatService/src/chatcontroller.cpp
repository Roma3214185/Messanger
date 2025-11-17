#include "chatservice/chatcontroller.h"

#include "NetworkManager.h"
#include "entities/User.h"
#include "NetworkManager.h"
#include "codes.h"
#include "interfaces/IConfigProvider.h"
#include "interfaces/IThreadPool.h"
#include "chatservice/interfaces/IChatManager.h"
#include "NetworkFacade.h"
#include "chatservice/AutoritizerProvider.h"

using std::optional;
using std::string;

namespace {

void sendResponse(crow::response& res, int code, const std::string& text) {
  res.code = code;
  res.write(text);
  res.end();
}

}  // namespace

ChatController::ChatController(IChatManager* manager,
                       NetworkFacade* network_facade, IConfigProvider* provider)
    : manager_(manager), network_facade_(network_facade), provider_(provider) {
  AutoritizerProvider::set(std::make_shared<RealAutoritizer>());
}

void ChatController::createPrivateChat(const crow::request& req, crow::response& res) {
  LOG_INFO("[temp] create private chat");
  optional<int> my_id = autoritize(req);
  if (!my_id) {
    sendResponse(res, provider_->statusCodes().userError, provider_->statusCodes().invalidToken);
    return;
  }

  auto body = crow::json::load(req.body);
  if (!body || !body.has("user_id")) {
    sendResponse(res, provider_->statusCodes().userError, "Missing user_id value");
    return;
  }

  int            user_id = body["user_id"].i();
  LOG_INFO("[temp] create private chat with user {}", user_id);
  optional<User> user    = getUserById(user_id);

  if (!user) {
    sendResponse(res, provider_->statusCodes().userError, provider_->statusCodes().userNotFound);
    return;
  }

  LOG_INFO("[temp] user finded {}", user->username);

  std::optional<int> chat_id = manager_->createPrivateChat(*my_id, user_id);
  if (!chat_id) {
    sendResponse(res, provider_->statusCodes().serverError, "Failed to create chat");
    return;
  }

  LOG_INFO("[CreatePrivateChat] Created chat with id '{}'", *chat_id);

  crow::json::wvalue result;
  result["id"] = *chat_id;
  result["type"]    = "private";
  result["user"]["id"]     = user->id;
  result["user"]["name"]   = user->username;
  result["user"]["avatar"] = user->avatar;

  sendResponse(res, provider_->statusCodes().success, result.dump());
}

std::optional<int> ChatController::autoritize(const crow::request& req) {
  string token   = req.get_header_value("Authorization");
  return AutoritizerProvider::get()->autoritize(token);
}

void ChatController::getAllChats(const crow::request& req, crow::response& res) {
  auto   user_id = authorizeUser(req, res);
  if (!user_id) return;

  auto chats = manager_->getChatsOfUser(*user_id);
  LOG_INFO("[GetAllChats] Returned chats '{}'", chats.size());

  crow::json::wvalue ans;
  ans["chats"] = crow::json::wvalue::list();

  int i = 0;
  for (const auto& chat : chats) {
    auto chat_json = buildChatJson(chat, *user_id);
    ans["chats"][i++] = std::move(chat_json);
  }

  sendResponse(res, provider_->statusCodes().success, ans.dump());
}

crow::json::wvalue ChatController::buildChatJson(const Chat& chat, int current_user_id) {
  crow::json::wvalue json;
  json["id"] = chat.id;
  json["type"] = chat.is_group ? "group" : "private";

  if (chat.is_group) {
    json["name"] = chat.name;
    json["avatar"] = chat.avatar;
    json["member_count"] = manager_->getMembersCount(chat.id);
  } else {
    auto other_user_id = manager_->getOtherMemberId(chat.id, current_user_id);
    if (other_user_id) {
      auto user = getUserById(*other_user_id);
      if (user) {
        json["user"]["id"] = user->id;
        json["user"]["name"] = user->username;
        json["user"]["avatar"] = user->avatar;
      } else {
        LOG_ERROR("[GetChat] Other user not found for chat '{}'", chat.id);
      }
    } else {
      LOG_ERROR("[GetChat] Can't find other members for chat '{}'", chat.id);
    }
  }

  return json;
}

void ChatController::getChat(const crow::request& req, crow::response& res, int chat_id) {
  auto user_id = authorizeUser(req, res);
  if (!user_id) return;

  auto chat_opt = manager_->getChatById(chat_id);
  if (!chat_opt) {
    sendError(res, provider_->statusCodes().userError, "Chat not found", "[GetChat] Chat not found");
    return;
  }

  const auto& chat = *chat_opt;
  auto chat_json = buildChatJson(chat, *user_id);
  sendResponse(res, provider_->statusCodes().success, chat_json.dump());
}

void ChatController::sendError(crow::response& res, int status, const std::string& message, const std::string& log_msg) {
  LOG_ERROR(log_msg);
  sendResponse(res, status, message);
}

std::optional<int> ChatController::authorizeUser(const crow::request& req, crow::response& res) {
  auto user_id = autoritize(req);
  if (!user_id) {
    LOG_ERROR("[GetChat] Can't verify token");
    sendResponse(res, provider_->statusCodes().userError, provider_->statusCodes().invalidToken);
  }
  return user_id;
}

void ChatController::getAllChatMembers(const crow::request& req, crow::response& res, int chat_id) {
  auto user_id = autoritize(req);
  if (!user_id) {
    LOG_ERROR("[getAllChatMembers] can't verify token");
    sendResponse(res, provider_->statusCodes().userError, provider_->statusCodes().invalidToken);
    return;
  }

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

std::optional<User> ChatController::getUserById(int id) {
  return network_facade_->user().getUserById(id);
}
