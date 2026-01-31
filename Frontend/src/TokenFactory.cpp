#include "MessageToken.h"

MessageToken TokenFactory::createTextToken(const QString& text) {
  MessageToken token;
  token.type = MessageTokenType::Text;
  token.value = text;
  return token;
}

MessageToken TokenFactory::createEmojiToken(long long emoji_id) {
  MessageToken token;
  token.type = MessageTokenType::Emoji;
  token.value = "{emoji:" + QString::number(emoji_id) + "}";
  token.emoji_id = emoji_id;
  return token;
}
