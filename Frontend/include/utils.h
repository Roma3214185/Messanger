#ifndef UTILS_H
#define UTILS_H

#include <QJsonObject>
#include <QString>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextFragment>
#include <optional>

#include "MessageToken.h"
#include "entities/ReactionInfo.h"

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

bool isSame(std::optional<long long> current_reaction, long long reaction_from_server);

}  // namespace utils

namespace utils::text {

namespace details {

void addImageToken(const QTextCharFormat& fmt, std::vector<MessageToken>& tokens);

void addToken(const QTextFragment& fragment, std::vector<MessageToken>& tokens);

}  // namespace details

std::vector<MessageToken> get_tokens_from_doc(const QTextDocument* doc);

QString tokenize(const std::vector<MessageToken>& tokens);

std::vector<MessageToken> get_tokens_from_text(const QString& text);

}  // namespace utils::text

#endif  // UTILS_H
