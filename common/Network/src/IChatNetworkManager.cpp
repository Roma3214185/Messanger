#include "interfaces/IChatNetworkManager.h"

#include <nlohmann/json.hpp>

#include "Debug_profiling.h"
#include "config/ports.h"
#include "config/codes.h"

std::vector<UserId> IChatNetworkManager::getMembersOfChat(long long chat_id) {
  const std::string path = "/chats/" + std::to_string(chat_id) + "/members";

  auto res = forward(Config::Ports::chatService, "", path, "GET");

  if (res.first != Config::StatusCodes::success) {
    LOG_ERROR("GetMembersOfChat failed '{}' reason: '{}'", res.first, res.second);
    return std::vector<UserId>{};
  }

  std::vector<UserId> members;
  try {
    nlohmann::json json_response = nlohmann::json::parse(res.second);

    if (!json_response.contains("members") || !json_response["members"].is_array()) {
      LOG_ERROR("getMembersOfChat: missing 'members' array");
      return std::vector<UserId>{};
    }

    for (const auto &variable : json_response["members"]) {
      if (variable.is_number_integer()) {
        members.push_back(variable.get<UserId>());
      }
    }

    LOG_INFO("getMembersOfChat success '{}'", members.size());
  } catch (const std::exception &e) {
    LOG_ERROR("JSON parse error in getMembersOfChat: {}", e.what());
    return std::vector<UserId>{};
  }

  return members;
}
