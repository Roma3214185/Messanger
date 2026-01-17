#ifndef UTILS_H
#define UTILS_H

#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <optional>
#include <QTextDocument>
#include <QTextFragment>
#include <QTextCursor>

#include "MessageToken.h"
#include "entities/ReactionInfo.h"
#include "Debug_profiling.h"

namespace utils {

template <typename T>
void add_optional_to_json(QJsonObject& json, const QString& json_field, const std::optional<T>& optional_value) {
  if (optional_value.has_value()) {
    json[json_field] = QJsonValue::fromVariant(QVariant::fromValue(*optional_value));
  } else {
    json[json_field] = QJsonValue::Null;
  }
}

template <typename T>
std::optional<T> get_optional_from_json(QJsonObject& json, const QString& key) {
  if (!json.contains(key)) return std::nullopt;

  const QJsonValue& value = json.value(key);

  if (value.isNull() || value.isUndefined()) return std::nullopt;

  QVariant variant = value.toVariant();

  if (!variant.canConvert<T>()) return std::nullopt;

  return variant.value<T>();
}

inline bool isSame(std::optional<long long> current_reaction, long long reaction_from_server) {
  return current_reaction.has_value() && current_reaction.value() == reaction_from_server;
}

}  // namespace utils

namespace utils::text {

namespace details {

inline void addImageToken(const QTextCharFormat& fmt, std::vector<MessageToken>& tokens) {
  if(!fmt.isImageFormat()) {
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

inline void addToken(const QTextFragment& fragment, std::vector<MessageToken>& tokens) {
  if(!fragment.isValid()) return;
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

inline std::vector<MessageToken> get_tokens_from_doc(const QTextDocument* doc) {
  if(!doc) return {};
  auto tokens = std::vector<MessageToken>{};

  for (auto b = doc->begin(); b != doc->end(); b = b.next()) {
    for (auto it = b.begin(); !it.atEnd(); ++it) {
      details::addToken(it.fragment(), tokens);
    }
  }

  return tokens;
}

inline QString tokenize(const std::vector<MessageToken>& tokens) {
  QString res;
  for(const auto& token: tokens) res += token.getText();
  return res;
}

inline std::vector<MessageToken> get_tokens_from_text(const QString& text) {
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
    QString emojiStr = text.mid(start + 7, end - (start + 7)); // skip "{emoji:"
    bool ok = false;
    long long emoji_id = emojiStr.toLongLong(&ok);
    if (ok) {
      tokens.push_back(TokenFactory::createEmojiToken(emoji_id));
    } else {
      // Malformed, treat as text
      tokens.push_back(TokenFactory::createTextToken(text.mid(start, end - start + 1)));
    }

    pos = end + 1; // move past this token
  }

  return tokens;
}

} // namespace utils::text


namespace utils::ui {

inline void insert_emoji(QTextCursor& cursor, std::optional<ReactionInfo> img_info_opt, int size_of_image = 16, QString default_value = " ") {
  if (img_info_opt.has_value()) {
    const ReactionInfo& info = img_info_opt.value();
    QTextImageFormat fmt;
    fmt.setName(QString::fromStdString(info.image));
    fmt.setWidth(size_of_image);
    fmt.setHeight(size_of_image);
    fmt.setProperty(QTextFormat::UserProperty, info.id);
    cursor.insertImage(fmt);
  } else {
    cursor.insertText(default_value);
  }
}

} // namespace utils::ui

#endif  // UTILS_H
