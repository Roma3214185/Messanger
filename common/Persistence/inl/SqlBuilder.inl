#pragma once

#include <nlohmann/json.hpp>

#include "Meta.h"
#include "SqlBuilder.h"

namespace {

template <typename T>
QVariant optionalToVariant(const std::optional<T>& opt) {
  return opt.has_value() ? QVariant::fromValue(opt.value()) : QVariant();
  // if (!opt.has_value())
  //   return QVariant(); // NULL
  // return QVariant::fromValue(*opt);
}

template <EntityJson T>
QVariant toVariant(const Field& f, const T& entity) {
  std::any val = f.get(&entity);

  if (f.type == typeid(long long))
    return QVariant::fromValue(std::any_cast<long long>(val));
  if (f.type == typeid(int))
    return  QVariant::fromValue(std::any_cast<int>(val));
  if (f.type == typeid(std::string))
    return QString::fromStdString(std::any_cast<std::string>(val));
  if (f.type == typeid(bool))
    return static_cast<int>(std::any_cast<bool>(val));
  if (f.type == typeid(int))
    return QVariant::fromValue(std::any_cast<int>(val));
  if (f.type == typeid(std::optional<long long>))
    return optionalToVariant(std::any_cast<std::optional<long long>>(val));

  LOG_ERROR("iNvalid toVariant {}", f.type.name());
  return {};
}

} // namespace
template <EntityJson T>
std::any SqlBuilder::getFieldValue(const QVariant& v, const Field& f) {
  if (f.type == typeid(std::optional<long long>)) {
    if (v.isNull())
      return std::any(std::optional<long long>{});
    return std::any(std::optional<long long>{v.toLongLong()});
  }

  if (!v.isValid()) {
    LOG_ERROR("v.isValid == false");
    return {};
  }

  //TODO: free function???

  if (f.type == typeid(long long))
    return std::any(v.toLongLong());

  if (f.type == typeid(std::string))
    return std::any(v.toString().toStdString());

  if (f.type == typeid(bool))
    return std::any(static_cast<bool>(v.toInt()));

  if (f.type == typeid(int))
    return std::any(v.toInt());

  LOG_ERROR("Unexpected typeid in getFieldValue {}", f.type.name());
  return {};
}

template <EntityJson T>
std::pair<QStringList, QStringList> buildInsertParts(
    const Meta& meta, const T& entity, QList<QVariant>& values) {
  QStringList cols, ph;
  for (const auto& f : meta.fields) {
    cols << f.name;
    ph << "?";
    values << toVariant<T>(f, entity);
  }
  return {cols, ph};
}

template<EntityJson T>
SqlStatement SqlBuilder::buildInsert(const Meta& meta, const T& entity) {
  SqlStatement res;
  auto values =  QList<QVariant>{};
  auto [columns, placeholders] = buildInsertParts(meta, entity, values);
  res.values = values;

  QString row_sql = QString("INSERT OR REPLACE INTO %1 (%2) VALUES (%3); ")
                 .arg(QString::fromStdString(meta.table_name))
                 .arg(columns.join(", "))
                 .arg(placeholders.join(", "));

  res.query = row_sql;
  return res;
}

template <EntityJson T>
T SqlBuilder::buildEntity(std::unique_ptr<IQuery>& query, const Meta& meta) const {
  if(!query) throw std::runtime_error("Nullptr in buildEntity"); //TODO: make NullptrObject
  T entity;
  for (const auto& f : meta.fields) {
    std::any val = SqlBuilder::getFieldValue<T>(query->value(f.name), f);
    f.set(&entity, val);
  }
  return entity;
}

template <EntityJson T>
std::vector<T> SqlBuilder::buildResults(std::unique_ptr<IQuery>& query) const {
  if(!query) return {};
  std::vector<T> results;
  auto meta = Reflection<T>::meta();

  while (query->next()) {
    T entity = buildEntity<T>(query, meta);
    LOG_INFO("Select {}", nlohmann::json(entity).dump());
    results.push_back(std::move(entity));
  }

  return results;
}
