#ifndef FIELDS_H
#define FIELDS_H

#include <string>

namespace MessageTable {

inline static constexpr const char *Table = "messages";
inline static constexpr const char *Id = "id";
inline static constexpr const char *ChatId = "chat_id";
inline static constexpr const char *Timestamp = "timestamp";
inline static constexpr const char *SenderId = "sender_id";
inline static constexpr const char *Text = "text";
inline static constexpr const char *LocalId = "local_id";

static std::string fullField(const std::string &field) { return std::string(Table) + "." + field; }

}  // namespace MessageTable

namespace MessageStatusTable {

inline static constexpr const char *Table = "messages_status";
inline static constexpr const char *MessageId = "message_id";
inline static constexpr const char *ReceiverId = "receiver_id";
inline static constexpr const char *IsRead = "is_read";
inline static constexpr const char *ReatAt = "read_at";

static std::string fullField(const std::string &field) { return std::string(Table) + "." + field; }

}  // namespace MessageStatusTable

namespace MessageReactionTable {

inline static constexpr const char *Table = "messages_reaction";
inline static constexpr const char *MessageId = "message_id";
inline static constexpr const char *ReceiverId = "receiver_id";
inline static constexpr const char *ReactionId = "reaction_id";

static std::string fullField(const std::string &field) { return std::string(Table) + "." + field; }

}  // namespace MessageReactionTable

namespace MessageReactionInfoTable {

inline static constexpr const char *Table = "messages_reaction_info";
inline static constexpr const char *Image = "image";
inline static constexpr const char *Id = "id";

static std::string fullField(const std::string &field) { return std::string(Table) + "." + field; }

}  // namespace MessageReactionInfoTable

namespace UserTable {

inline static constexpr const char *Table = "users";
inline static constexpr const char *TableByEmail = "users_by_email";
inline static constexpr const char *Id = "id";
inline static constexpr const char *Username = "username";
inline static constexpr const char *Tag = "tag";
inline static constexpr const char *Email = "email";

static std::string fullField(const std::string &field) { return std::string(Table) + "." + field; }

}  // namespace UserTable

namespace UserCredentialsTable {

inline static constexpr const char *Table = "credentials";
inline static constexpr const char *UserId = "user_id";
inline static constexpr const char *HashPassword = "hash_password";

static std::string fullField(const std::string &field) { return std::string(Table) + "." + field; }

}  // namespace UserCredentialsTable

namespace ChatMemberTable {

inline static constexpr const char *Table = "chat_members";
inline static constexpr const char *ChatId = "chat_id";
inline static constexpr const char *UserId = "user_id";
inline static constexpr const char *Status = "status";
inline static constexpr const char *AddedAt = "added_at";

static std::string fullField(const std::string &field) { return std::string(Table) + "." + field; }

}  // namespace ChatMemberTable

namespace ChatTable {

inline static constexpr const char *Table = "chats";
inline static constexpr const char *Id = "id";
inline static constexpr const char *IsGroup = "is_group";
inline static constexpr const char *Name = "name";
inline static constexpr const char *Avatar = "avatar";
inline static constexpr const char *CreatedAt = "created_at";

static std::string fullField(const std::string &field) { return std::string(Table) + "." + field; }

}  // namespace ChatTable

namespace PrivateChatTable {

inline static constexpr const char *Table = "private_chats";
inline static constexpr const char *ChatId = "chat_id";
inline static constexpr const char *FirstUserId = "user1_id";
inline static constexpr const char *SecondUserId = "user2_id";

static std::string fullField(const std::string &field) { return std::string(Table) + "." + field; }

}  // namespace PrivateChatTable

#endif  // FIELDS_H
