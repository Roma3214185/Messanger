#ifndef MESSAGETOKEN_H
#define MESSAGETOKEN_H

#include <QString>
#include <optional>

#include "entities/ReactionInfo.h"

enum class MessageTokenType : std::uint8_t { Text, Emoji };

struct MessageToken {
  MessageTokenType type;
  QString value;
  std::optional<long long> emoji_id;

  QString getText() const { return value; }

  bool operator==(const MessageToken& other) const = default;
};

struct TokenFactory {
  static MessageToken createTextToken(const QString& text);
  static MessageToken createEmojiToken(long long emoji_id);
};

#endif  // MESSAGETOKEN_H
