#pragma once

#include <nlohmann/json.hpp>

#include "query/SelectQuery.h"
#include "SqlExecutor.h"
#include "Meta.h"
#include "SqlBuilder.h"

namespace {

std::size_t hashParams(const  QVector<QVariant>& values) {
  std::size_t params_hash = 0;
  for (const auto& v : values)
    params_hash ^= std::hash<std::string>{}(v.toString().toStdString());
  return params_hash;
}

using Table = std::string;
std::size_t hashGenerations(
    const std::unordered_map<Table, std::string>& generations) {
  std::size_t generation_hash = 0;
  for (const auto& [table, gen] : generations)
    generation_hash ^= std::hash<std::string>{}(table + gen);
  return generation_hash;
}

std::string buildEntityKey(const std::string& table_name, const std::string& entity_key) {
  return "entity_cache:" + table_name + ":" + entity_key;
}

std::string createCacheKey(const QString& sql, int generation_hash,
                           int params_hash) {
  return "query_cache:" + sql.toStdString() +
         ":gen=" + std::to_string(generation_hash) +
         ":params=" + std::to_string(params_hash);
}

}  // namespace

template <EntityJson T>
std::optional<std::vector<T>> SelectQuery<T>::tryLoadFromCache(const std::string& key) const {
  if (std::optional<std::string> entity_string = cache_.get(key)) {
    LOG_INFO("[QueryCache] HIT for key '{}'", key);
    try {
      nlohmann::json json_obj = nlohmann::json::parse(*entity_string);
      auto res = json_obj.template get<std::vector<T>>();
      LOG_INFO("[QueryCache] Parsed successfully: '{}'", res.size());
      return res;
    } catch (...) {
      LOG_WARN("[QueryCache] Failed to parse cached data for key '{}'", key);
    }
  } else {
    LOG_INFO("[QueryCache] MISS for key '{}'", key);
  }
  return std::nullopt;
}

template <EntityJson T>
SelectQuery<T>::SelectQuery(ISqlExecutor* executor, ICacheService& cache)
    : IBaseQuery<T>(executor), cache_(cache) { }

template <EntityJson T>
QueryResult<T> SelectQuery<T>::execute() const {
  QString sql = buildQuery();
  std::string cache_key = createCacheKey(sql, hashGenerations(getGenerations()), hashParams(this->values_));

  if (auto cached = tryLoadFromCache(cache_key)) {
    LOG_INFO("Hit cache for key {}", cache_key);
    return SelectResult<T>{ *cached };
  }

  LOG_INFO("Not hit cache for sql {}", sql.toStdString());

  auto query = this->executor_->execute(sql, this->values_);
  if(!query) {
    LOG_ERROR("query {} failed", sql.toStdString());
    return SelectResult<T>{ std::vector<T>{ } };
  }

  LOG_INFO("query {} succeed", sql.toStdString());
  auto results = builder_.buildResults(query);
  updateCache(cache_key, results);

  LOG_INFO("Results has {} size", results.size());
  return SelectResult<T>{ results };
}

template <EntityJson T>
void SelectQuery<T>::updateCache(const std::string& key, const std::vector<T>& results) const {
  std::vector<std::string> entities_strings;
  std::vector<std::string> entities_keys;

  for (const auto& entity : results) {
    entities_strings.push_back(nlohmann::json(entity).dump());
    std::string entity_key = EntityKey<T>::get(entity);
    entities_keys.push_back(buildEntityKey(this->table_name_.toStdString(), entity_key));
  }

  cache_.setPipelines(entities_keys, entities_strings, std::chrono::seconds(30));
  cache_.set(key, nlohmann::json(results).dump(), std::chrono::seconds(30));
}

/*
 todo:
  std::string entity_key = EntityKey<T>::get(entity);
  std::string key = buildEntityKey(this->table_name_.toStdString(), entity_key);

  std::string key = getEntityKey {
    std::string entity_key = EntityKey<T>::get(entity);
    return buildEntityKey(this->table_name_.toStdString(), entity_key);
  };
*/

template <EntityJson T>
void SelectQuery<T>::saveEntityInCache(
    const T& entity, std::chrono::hours ttl) const {
  std::string entity_key = EntityKey<T>::get(entity);
  std::string key = buildEntityKey(this->table_name_.toStdString(), entity_key);
  cache_.set(key, entity, ttl);
}

template <EntityJson T>
int SelectQuery<T>::getEntityId(const T& entity) const { return entity.id; }

template <EntityJson T>
QString SelectQuery<T>::buildQuery() const {
  QString sql =
      QString("SELECT * FROM %1").arg(this->table_name_);
  sql += this->join_clause_;
  if (!this->filters_.empty()) sql += " WHERE " + this->filters_.join(" AND ");
  if (!this->order_.isEmpty()) sql += " " + this->order_;
  if (!this->limit_clause_.isEmpty()) sql += " " + this->limit_clause_;
  return sql;
}

template <EntityJson T>
auto SelectQuery<T>::getGenerations() const {
  std::unordered_map<std::string, std::string> generations;
  for (const auto& table : this->involved_tables_) {
    std::string key = std::string("table_generation:") + table.toStdString();
    std::optional<std::string> string_ptr = cache_.get(key);
    generations[table.toStdString()] = string_ptr ? *string_ptr : "0";
  }
  return generations;
}

template <EntityJson T>
SelectQuery<T>& SelectQuery<T>::orderBy(const std::string& field,
                            const OrderDirection& direction) & {
  const QString direct = direction == OrderDirection::ASC ? "ASC" : "DESC";
  order_ = QString("ORDER BY %1 %2")
    .arg(QString::fromStdString(field))
    .arg(direct);
  return *this;
}
