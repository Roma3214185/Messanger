#ifndef MESSAGETOKEN_H
#define MESSAGETOKEN_H

#include <QString>
#include "entities/ReactionInfo.h"

enum class MessageTokenType {
  Text,
  Emoji
};

struct MessageToken {
    MessageTokenType type;
    QString value;
    std::optional<long long> emoji_id;

    QString getText() const { return value; }

    bool operator==(const MessageToken& other) const {
      return type == other.type &&
             value == other.value &&
             emoji_id == other.emoji_id;
    }
};

struct TokenFactory {
    static MessageToken createTextToken(const QString& text)  {
      MessageToken token;
      token.type = MessageTokenType::Text;
      token.value = text;
      return token;
    }

    static MessageToken createEmojiToken(long long emoji_id)  {
      MessageToken token;
      token.type = MessageTokenType::Emoji;
      token.value = "{emoji:" + QString::number(emoji_id) + "}";
      token.emoji_id = emoji_id;
      return token;
    }
};

#endif // MESSAGETOKEN_H
