#include "utils.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>
#include <optional>

#include "Debug_profiling.h"
#include "MessageToken.h"
#include "entities/ReactionInfo.h"

namespace utils {

bool isSame(std::optional<long long> current_reaction, long long reaction_from_server) {
  return current_reaction.has_value() && current_reaction.value() == reaction_from_server;
}

}  // namespace utils

namespace utils::text {

namespace details {

void addImageToken(const QTextCharFormat& fmt, std::vector<MessageToken>& tokens) {
  if (!fmt.isImageFormat()) {
    DBC_UNREACHABLE();
    return;
  }

  QTextImageFormat imgFmt = fmt.toImageFormat();

  QVariant v = imgFmt.property(QTextFormat::UserProperty);

  if (v.isValid()) {
    long long emoji_id = v.toLongLong();
    auto token = TokenFactory::createEmojiToken(emoji_id);
    tokens.push_back(token);
    LOG_INFO("Custom emoji with id ", emoji_id);
  }
}

void addToken(const QTextFragment& fragment, std::vector<MessageToken>& tokens) {
  if (!fragment.isValid()) return;
  QTextCharFormat fmt = fragment.charFormat();

  if (fmt.isImageFormat()) {
    details::addImageToken(fmt, tokens);
  } else {
    QString text = fragment.text();
    auto token = TokenFactory::createTextToken(text);
    tokens.push_back(token);
    LOG_INFO("Text: ", text.toStdString());
  }
}

}  // namespace details

std::vector<MessageToken> get_tokens_from_doc(const QTextDocument* doc) {
  if (!doc) return {};
  auto tokens = std::vector<MessageToken>{};

  for (auto b = doc->begin(); b != doc->end(); b = b.next()) {
    for (auto it = b.begin(); !it.atEnd(); ++it) {
      details::addToken(it.fragment(), tokens);
    }
  }

  return tokens;
}

QString tokenize(const std::vector<MessageToken>& tokens) {
  QString res;
  for (const auto& token : tokens) res += token.getText();
  return res;
}

std::vector<MessageToken> get_tokens_from_text(const QString& text) {
  std::vector<MessageToken> tokens;
  int pos = 0;

  while (pos < text.length()) {
    int start = static_cast<int>(text.indexOf("{emoji:", pos));
    if (start == -1) {
      // No more emojis, take the rest as text
      if (pos < text.length()) {
        QString t = text.mid(pos);
        tokens.push_back(TokenFactory::createTextToken(t));
      }
      break;
    }

    // Text before emoji
    if (start > pos) {
      QString t = text.mid(pos, start - pos);
      tokens.push_back(TokenFactory::createTextToken(t));
    }

    // Find end of emoji token
    int end = static_cast<int>(text.indexOf('}', start));
    if (end == -1) {
      // Malformed, take the rest as text
      QString t = text.mid(start);
      tokens.push_back(TokenFactory::createTextToken(t));
      break;
    }

    // Extract emoji id
    QString emojiStr = text.mid(start + 7, end - (start + 7));  // skip "{emoji:"
    bool ok = false;
    long long emoji_id = emojiStr.toLongLong(&ok);
    if (ok) {
      tokens.push_back(TokenFactory::createEmojiToken(emoji_id));
    } else {
      // Malformed, treat as text
      tokens.push_back(TokenFactory::createTextToken(text.mid(start, end - start + 1)));
    }

    pos = end + 1;  // move past this token
  }

  return tokens;
}

}  // namespace utils::text
