#include "interfaces/IChatNetworkManager.h"
#include <nlohmann/json.hpp>

#include "Debug_profiling.h"

using json = nlohmann::json;

std::vector<UserId> IChatNetworkManager::getMembersOfChat(long long chat_id) {
  std::vector<UserId> members;
  std::string path = "/chats/" + std::to_string(chat_id) + "/members";

  auto res = forward(provider_->ports().chatService, "", path, "GET");

  if (res.first != provider_->statusCodes().success) {
    LOG_ERROR("GetMembersOfChat failed '{}' reason: '{}'", res.first, res.second);
    return members;
  }

  try {
    json json_response = json::parse(res.second);

    if (!json_response.contains("members") || !json_response["members"].is_array()) {
      LOG_ERROR("getMembersOfChat: missing 'members' array");
      return members;
    }

    for (const auto& v : json_response["members"]) {
      if (v.is_number_integer()) {
        members.push_back(v.get<UserId>());
      }
    }

    LOG_INFO("getMembersOfChat success '{}'", members.size());
  }
  catch (const std::exception& e) {
    LOG_ERROR("JSON parse error in getMembersOfChat: {}", e.what());
  }

  return members;
}
