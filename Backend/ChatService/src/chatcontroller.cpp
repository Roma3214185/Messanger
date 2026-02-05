#include "chatservice/chatcontroller.h"

#include "Debug_profiling.h"
#include "NetworkFacade.h"
#include "chatservice/interfaces/IChatManager.h"
#include "config/codes.h"
#include "entities/ReactionInfo.h"
#include "entities/RequestDTO.h"
#include "entities/User.h"
#include "interfaces/IAutoritizer.h"

using std::optional;
using std::string;

namespace {

template <typename T>
inline void append(crow::json::wvalue &arr, const T &item) {
  auto idx = arr.size();
  arr[idx] = item;
}

std::optional<long long> getIdFromStr(const std::string &str) {
  try {
    return std::stoll(str);
  } catch (...) {
    LOG_ERROR("Error while get stoll of {}", str);
    return std::nullopt;
  }
}

[[nodiscard]] Response sendResponse(int code, const std::string &text) { return std::make_pair(code, text); }

[[nodiscard]] nlohmann::json buildChatJson(const Chat &chat,  // todo: make entity that fully handles all required info
                                           const std::vector<ReactionInfo> &reactions,
                                           const std::optional<User> &other_user, std::optional<int> member_count) {
  nlohmann::json json;
  json["id"] = chat.id;
  json["type"] = chat.is_group ? "group" : "private";

  if (chat.is_group) {  // todo: implement factory create(Private/Group)ChatJson()
    json["name"] = chat.name;
    json["avatar"] = chat.avatar;
    json["member_count"] = member_count.value_or(0);
  } else if (other_user.has_value()) {
    json["user"]["id"] = other_user->id;
    json["user"]["name"] = other_user->username;
    json["user"]["avatar"] = other_user->avatar;
  }

  json["default_reactions"] = reactions;

  return json;
}

}  // namespace

ChatController::ChatController(IChatManager *manager, INetworkFacade *network_facade, IAuthoritizer *authoritizer)
    : manager_(manager), network_facade_(network_facade), authoritizer_(authoritizer) {}

Response ChatController::createPrivateChat(const RequestDTO &req) {
  auto my_id = autoritize(req.token);
  if (!my_id) {
    return sendResponse(Config::StatusCodes::unauthorized,
                        utils::details::formError(Config::IssueMessages::invalidToken));
  }

  auto body = crow::json::load(req.body);
  if (!body || !body.has("user_id")) {
    if (!body) {
      LOG_ERROR("Invalid body");
    } else {
      LOG_ERROR("Missing 'user_id' value");
    }
    return sendResponse(Config::StatusCodes::userError, utils::details::formError("Missing user_id value"));
  }

  auto user_id = [body]() -> std::optional<long long> {
    if (body["user_id"].t() == crow::json::type::String) {
      return std::stoll(body["user_id"].s());
    }

    if (body["user_id"].t() == crow::json::type::Number) {
      return static_cast<long long>(body["user_id"].d());
    }

    return std::nullopt;
  }();

  if (!user_id.has_value()) {
    LOG_ERROR("Can't get 'user_id' from json");
    return sendResponse(Config::StatusCodes::badRequest, utils::details::formError("User_id invalid"));
  }

  LOG_INFO("Create private chat with {} and user {}", *my_id, *user_id);
  optional<User> user = getUserById(*user_id);

  if (!user) {
    LOG_ERROR("User with id {} not found", *user_id);
    return sendResponse(Config::StatusCodes::userError, utils::details::formError(Config::IssueMessages::userNotFound));
  }

  LOG_INFO("User finded {}", nlohmann::json(user).dump());

  std::optional<long long> chat_id = manager_->createPrivateChat(*my_id, *user_id);
  if (!chat_id.has_value()) {
    LOG_ERROR("Failed to create private chat for users '{}, {}'", *my_id, *user_id);
    return sendResponse(Config::StatusCodes::serverError, utils::details::formError("Failed to create chat"));
  }

  LOG_INFO("[CreatePrivateChat] Created chat with id '{}'", *chat_id);

  Chat chat;
  chat.id = *chat_id;
  chat.is_group = 0;
  chat.name = "test_name";
  auto reactions = getReactionOfChat(chat.id);
  auto result = buildChatJson(chat, reactions, user, 0);
  return sendResponse(Config::StatusCodes::success, result.dump());
}

std::optional<long long> ChatController::autoritize(const std::string &token) {
  return authoritizer_->verifyTokenAndGetUserId(token);
}

Response ChatController::getAllChats(const RequestDTO &req) {
  LOG_INFO("Get all chats, req is {}", req.body);
  auto user_id = autoritize(req.token);
  if (!user_id.has_value()) {
    return sendResponse(Config::StatusCodes::userError, utils::details::formError(Config::IssueMessages::invalidToken));
  }

  auto chats = manager_->getChatsIdOfUser(*user_id);
  LOG_INFO("For user_id {} finded {} chats", *user_id, chats.size());

  crow::json::wvalue ans;
  ans["chats"] = crow::json::wvalue::list();

  for (const auto &chat_id : chats) {
    LOG_INFO("User id {} - chat_id {}", *user_id, chat_id);
    auto [code, body] = getChat(req, std::to_string(chat_id));
    LOG_INFO("For chat {} finded res is {}", chat_id, body);

    if (code == Config::StatusCodes::success) {
      append(ans["chats"], crow::json::load(body));
    } else {
      LOG_ERROR("[GetAllChats] Failed to retrieve chat with ID '{}'", chat_id);
      // return sendError(res, chat_res.code, chat_res.body);
    }
  }

  return sendResponse(Config::StatusCodes::success, ans.dump());
}

Response ChatController::getChat(const RequestDTO &req, const std::string &chat_id_str) {
  auto user_id = autoritize(req.token);
  if (!user_id.has_value()) {
    return sendResponse(Config::StatusCodes::unauthorized,
                        utils::details::formError(Config::IssueMessages::invalidToken));
  }

  LOG_INFO("chat_id_str = {}", chat_id_str);
  auto chat_id = getIdFromStr(chat_id_str);
  if (!chat_id.has_value()) {
    return sendResponse(Config::StatusCodes::badRequest, utils::details::formError("Invalid id"));
  }

  LOG_INFO("Chat id is {}", *chat_id);

  auto chat_opt = manager_->getChatById(*chat_id);
  if (!chat_opt) {
    return sendResponse(Config::StatusCodes::userError, utils::details::formError("Chat not found"));
  }

  const auto &chat = *chat_opt;
  if (chat.is_group) {
    auto count = manager_->getMembersCount(chat.id);

    if (count <= 0) {
      return sendResponse(Config::StatusCodes::serverError,
                          utils::details::formError("Failed to retrieve group member count"));
    }
    auto reactions = getReactionOfChat(chat.id);
    auto chat_json = buildChatJson(chat, reactions, std::nullopt, count);
    return sendResponse(Config::StatusCodes::success, chat_json.dump());
  }

  auto other_user_id = manager_->getOtherMemberId(chat.id, *user_id);
  if (!other_user_id.has_value()) {
    return sendResponse(Config::StatusCodes::notFound, utils::details::formError("Other user not found for this chat"));
  }

  auto other_user = getUserById(*other_user_id);
  if (!other_user) {
    return sendResponse(Config::StatusCodes::badRequest, utils::details::formError("User profile not found"));
  }
  auto reactions = getReactionOfChat(chat.id);
  auto chat_json = buildChatJson(chat, reactions, other_user, std::nullopt);
  return sendResponse(Config::StatusCodes::success, chat_json.dump());
}

Response ChatController::getAllChatMembers(const RequestDTO & /*req*/, const std::string &chat_id_str) {
  LOG_INFO("chat_id_str = {}", chat_id_str);
  std::optional<long long> chat_id = getIdFromStr(chat_id_str);
  if (!chat_id.has_value()) {
    return sendResponse(Config::StatusCodes::badRequest, utils::details::formError("Invalid id"));
  }

  LOG_INFO("Chat id is {}", *chat_id);

  auto list_of_members = manager_->getMembersOfChat(*chat_id);
  if (list_of_members.empty()) {
    LOG_ERROR("[GetAllChatsMembers] Error in db.getMembersOfChat");
    return sendResponse(Config::StatusCodes::serverError, utils::details::formError("Error in db.getMembersOfChat"));
  }

  LOG_INFO("Db return '{}' members for chat '{}'", list_of_members.size(), *chat_id);
  crow::json::wvalue ans;
  ans["members"] = crow::json::wvalue::list();

  for (auto member_id : list_of_members) {
    LOG_INFO("ChatId '{}' - member '{}'", *chat_id, member_id);
    append(ans["members"], member_id);
  }

  return sendResponse(Config::StatusCodes::success, ans.dump());
}

std::optional<User> ChatController::getUserById(long long id) { return network_facade_->users().getUserById(id); }

std::vector<ReactionInfo> ChatController::getReactionOfChat(long long chat_id) {
  DBC_REQUIRE(chat_id > 0);
  std::vector<long long> ids_of_reactions{
      1, 2};  // todo: make table ChatReactions chat_id | reaction_id with default values

  std::vector<ReactionInfo> reactions_of_chat;
  for (const auto &id_of_reactions : ids_of_reactions) {
    if (auto reaction = network_facade_->messages().getReaction(id_of_reactions); reaction.has_value()) {
      DBC_REQUIRE(reaction->checkInvariants());
      reactions_of_chat.push_back(reaction.value());
    } else {
      LOG_ERROR("Failed to get reaction {} for chat {}", id_of_reactions, chat_id);
    }
  }
  return reactions_of_chat;
}
