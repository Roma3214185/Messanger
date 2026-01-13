#ifndef BACKEND_CHATSERVICE_SRC_HEADERS_CHAT_H_
#define BACKEND_CHATSERVICE_SRC_HEADERS_CHAT_H_

#include <nlohmann/json.hpp>
#include <string>

#include "Debug_profiling.h"
#include "Fields.h"
#include "TimestampService.h"
#include "interfaces/entity.h"

struct Chat final {
  long long id{0};
  int is_group{0};
  std::string name;
  std::string avatar;
  long long created_at{0};

  Chat() = default;

  Chat(long long id, int is_group, std::string name, std::string avatar,
       long long created_at = utils::time::getCurrentTime())
      : id(id), is_group(is_group), name(std::move(name)), avatar(std::move(avatar)), created_at(created_at) {
    DBC_REQUIRE(checkInvariants());
  }

  bool checkInvariants() const { return id > 0; } //&& !name.empty(); }
};

namespace nlohmann {

template <>
struct adl_serializer<Chat> {
  static void to_json(nlohmann::json &json_chat, const Chat &chat) {
    json_chat = nlohmann::json{{ChatTable::Id, chat.id},
                               {ChatTable::IsGroup, chat.is_group},
                               {ChatTable::Name, chat.name},
                               {ChatTable::Avatar, chat.avatar},
                               {ChatTable::CreatedAt, chat.created_at}};
  }

  static void from_json(const nlohmann::json &json_chat, Chat &chat) {
    json_chat.at(ChatTable::Id).get_to(chat.id);
    json_chat.at(ChatTable::IsGroup).get_to(chat.is_group);
    json_chat.at(ChatTable::Name).get_to(chat.name);
    json_chat.at(ChatTable::Avatar).get_to(chat.avatar);
    json_chat.at(ChatTable::CreatedAt).get_to(chat.created_at);
  }
};

}  // namespace nlohmann

#endif  // BACKEND_CHATSERVICE_SRC_HEADERS_CHAT_H_
