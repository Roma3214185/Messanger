#ifndef REACTIONINFO_H
#define REACTIONINFO_H

#include <string>
#include <nlohmann/json.hpp>

struct ReactionInfo {
  long long id;
  std::string image;

  bool checkInvariants() const {
    return id > 0 && !image.empty();
  }
};

inline bool operator==(const ReactionInfo& a, const ReactionInfo& b) {
  return a.id == b.id &&
         a.image == b.image;
}

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

namespace std {

template<>
struct hash<ReactionInfo> {
    size_t operator()(const ReactionInfo& r) const noexcept {
      //size_t h1 = std::hash<int>{}(r.id);
      //return h1 ^ (h2 << 1); // combine
      return std::hash<long long>{}(r.id);
    }
};
}  // namespace std

#endif // REACTIONINFO_H
