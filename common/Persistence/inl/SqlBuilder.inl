#pragma once

#include "Meta.h"

template <typename T>
QVariant toVariant(const Field& f, const T& entity) {
  std::any val = f.get(&entity);

  if (f.type == typeid(long long))
    return QVariant::fromValue(std::any_cast<long long>(val));
  if (f.type == typeid(std::string))
    return QString::fromStdString(std::any_cast<std::string>(val));
  if (f.type == typeid(bool))
    return static_cast<int>(std::any_cast<bool>(val));

  return {};
}

template <typename T>
std::any SqlBuilder<T>::getFieldValue(const QVariant& v, const Field& f) {
  if (!v.isValid())
    return {};

  if (f.type == typeid(long long))
    return std::any(v.toLongLong());

  if (f.type == typeid(std::string))
    return std::any(v.toString().toStdString());

  if (f.type == typeid(bool))
    return std::any(static_cast<bool>(v.toInt()));

  return {};
}

template <typename T>
std::pair<QStringList, QStringList> buildInsertParts(
    const Meta& meta, const T& entity, QList<QVariant>& values) {
  QStringList cols, ph;
  for (const auto& f : meta.fields) {
    if (std::string(f.name) == "id" && toVariant<T>(f, entity) == 0) continue;

    cols << f.name;
    ph << "?";
    values << toVariant<T>(f, entity);
  }
  return {cols, ph};
}

template<typename T>
SqlStatement SqlBuilder<T>::buildInsert(const Meta& meta, const T& entity, bool need_to_return_id) {
  SqlStatement res;
  auto values =  QList<QVariant>{};
  auto [columns, placeholders] = buildInsertParts(meta, entity, values);
  res.values = values;

  QString row_sql = "INSERT OR REPLACE INTO %1 (%2) VALUES (%3)";
  if(need_to_return_id) row_sql += " RETURNING id";

  res.query = row_sql
              .arg(QString::fromStdString(meta.table_name))
              .arg(columns.join(", "))
              .arg(placeholders.join(", "));
  return res;
}
