#ifndef REACTIONINFO_H
#define REACTIONINFO_H

#include <string>
#include <nlohmann/json.hpp>

struct ReactionInfo {
  long long id;
  std::string image;

  bool operator ==(const ReactionInfo& other) {
    return other.id == id;
  }

  ReactionInfo& operator =(const ReactionInfo& other) {
    if(*this != other) {
      id = other.id;
      image = other.image;
    }
    return *this;
  }
};


namespace nlohmann {

template <>
struct adl_serializer<ReactionInfo> {
    static void to_json(nlohmann::json &json_message, const ReactionInfo &user_reaction) {
      json_message["image"] = user_reaction.image;
      json_message["id"] = user_reaction.id;
    }

    static void from_json(const nlohmann::json &json_message, ReactionInfo &user_reaction) {
      user_reaction.image = json_message["image"];
      user_reaction.id = json_message["id"];
    }
};

}  // namespace nlohmann

#endif // REACTIONINFO_H
