#pragma once

#include <nlohmann/json.hpp>

#include "Query.h"
#include "SqlExecutor.h"
#include "Meta.h"
#include "SqlBuilder.h"

template <typename T>
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

template <typename T>
SelectQuery<T>::SelectQuery(ISqlExecutor* executor, ICacheService& cache)
    : BaseQuery<T>(executor), cache_(cache) { }

template <typename T>
std::vector<T> SelectQuery<T>::execute() const {
  QString sql = buildSelectQuery();
  std::string cache_key = computeCacheKey(sql);
  if (auto cached = tryLoadFromCache(cache_key)) {
    LOG_INFO("Hit cache for key {}", cache_key);
    return *cached;
  }

  LOG_INFO("Not hit cache for key {}", cache_key);
  LOG_INFO("Try to execute {}", sql.toStdString());
  if(!this->executor_) {
    LOG_ERROR("Executor is null");
    return std::vector<T>{};
  }
  auto query = this->executor_->execute(sql, this->values_);
  if(!query) {
    LOG_ERROR("query {} failed", sql.toStdString());
    return std::vector<T>{};
  }
  LOG_INFO("query {} succeed", sql.toStdString());
  auto results = buildResults(query);
  LOG_INFO("Results has {} size", results.size());
  updateCache(cache_key, results);

  return results;
}

// template <typename T>
// QSqlQuery SelectQuery<T>::runDatabaseQuery(const QString& sql) const {
//   QSqlQuery query;
//   if (!this->executor_->execute(sql, this->values_)) {
//     //LOG_ERROR("[QueryExecutor] SQL execution failed: '{}'", sql.toStdString());
//   }
//   return query;
// }

template <typename T>
std::vector<T> SelectQuery<T>::buildResults(std::unique_ptr<IQuery>& query) const {
  if(!query) {
    LOG_ERROR("Query in null");
    return {};
  }
  std::vector<T> results;
  auto meta = Reflection<T>::meta();

  while (query->next()) {
    T entity = buildEntity(query, meta);
    LOG_INFO("Selected {}", nlohmann::json(entity).dump());
    results.push_back(std::move(entity));
  }

  return results;
}

template <typename T>
void SelectQuery<T>::updateCache(const std::string& key, const std::vector<T>& results) const {
  std::vector<std::string> entities_strings;
  std::vector<std::string> entities_keys;

  for (const auto& entity : results) {
    entities_strings.push_back(nlohmann::json(entity).dump());
    entities_keys.push_back(buildEntityKey(entity));
  }

  nlohmann::json j = results;
  std::string serialized = j.dump();
  cache_.setPipelines(entities_keys, entities_strings);
  cache_.set(key, serialized, std::chrono::hours(24));
}

template <typename T>
std::string SelectQuery<T>::computeCacheKey(const QString& sql) const {
  LOG_INFO("computeCacheKey start");
  auto generations = getGenerations();
  auto generation_hash = hashGenerations(generations);
  auto params_hash = hashParams(this->values_);
  return createCacheKey(sql, generation_hash, params_hash);
}

template <typename T>
std::future<std::vector<T>> SelectQuery<T>::executeAsync() const {
  return this->pool.enqueue([this]() { return execute(); });
}

template <typename T>
std::future<std::vector<T>> SelectQuery<T>::executeWithoutCacheAsync() const {
  return this->pool.enqueue([this]() {
    return executeWithoutCache();  // DB initialized in this thread
  });
}

template <typename T>
std::vector<T> SelectQuery<T>::executeWithoutCache() const {
  // QString sql = buildSelectQuery();
  // QSqlDatabase threadDb =
  //     this->db_.getThreadDatabase();

  // QSqlQuery query(threadDb);
  // query.prepare(sql);
  // for (int i = 0; i < this->values_.size(); ++i) query.bindValue(i, this->values_[i]);

  // if (!query.exec()) {
  //   LOG_ERROR("Query error: {}", query.lastError().text().toStdString());
  //   return {};
  // }

  // std::vector<T> results;
  // auto meta = Reflection<T>::meta();
  // while (query.next()) {
  //   results.push_back(buildEntity(query, meta));
  // }

  // LOG_INFO("Result size is '{}'", results.size());
  // return results;
}

template <typename T>
void SelectQuery<T>::saveEntityInCache(
    const T& entity, std::chrono::hours ttl) const {
  std::string entityKey = buildEntityKey(entity);
  cache_.set(entityKey, entity, ttl);
}

template <typename T>
T SelectQuery<T>::buildEntity(std::unique_ptr<IQuery>& query, const Meta& meta) const { //TODO(roma): in builder
  if(!query) throw std::runtime_error("Nullptr in buildEntity"); //TODO: make NullptrObject
  T entity;
  for (const auto& f : meta.fields) {
    std::any val = SqlBuilder<T>::getFieldValue(query->value(f.name), f);
    f.set(&entity, val);
  }
  return entity;
}

template <typename T>
int SelectQuery<T>::getEntityId(const T& entity) const { return entity.id; }

template <typename T>
QString SelectQuery<T>::buildSelectQuery() const {
  QString sql =
      QString("SELECT * FROM %1").arg(this->table_name_);
  sql += this->join_clause_;
  if (!this->filters_.empty()) sql += " WHERE " + this->filters_.join(" AND ");
  if (!this->order_.isEmpty()) sql += " " + this->order_;
  if (!this->limit_clause_.isEmpty()) sql += " " + this->limit_clause_;
  return sql;
}

template <typename T>
auto SelectQuery<T>::getGenerations() const {
  std::unordered_map<std::string, std::string> generations;
  for (const auto& table : this->involved_tables_) {
    std::string key = std::string("table_generation:") + table.toStdString();
    std::optional<std::string> string_ptr = cache_.get(key);
    generations[table.toStdString()] = string_ptr ? *string_ptr : "0";
  }
  return generations;
}

using Table = std::string;

template <typename T>
std::size_t SelectQuery<T>::hashGenerations(
    const std::unordered_map<Table, std::string>& generations) const {
  std::size_t generation_hash = 0;
  for (const auto& [table, gen] : generations)
    generation_hash ^= std::hash<std::string>{}(table + gen);
  return generation_hash;
}

template <typename T>
SelectQuery<T>& SelectQuery<T>::orderBy(const std::string& field,
                            const OrderDirection& direction) {
  QString direct = direction == OrderDirection::ASC ? "ASC" : "DESC";
  order_ = QString("ORDER BY %1 %2")
    .arg(QString::fromStdString(field))
    .arg(direct);
  return *this;
}

template <typename T>
std::size_t SelectQuery<T>::hashParams(const  QVector<QVariant>& values) const {
  std::size_t params_hash = 0;
  for (const auto& v : values)
    params_hash ^= std::hash<std::string>{}(v.toString().toStdString());
  return params_hash;
}

template <typename T>
std::string SelectQuery<T>::buildEntityKey(const T& entity) const {
  return "entity_cache:" + this->table_name_.toStdString() + ":" + EntityKey<T>::get(entity);
}

template <typename T>
std::string SelectQuery<T>::createCacheKey(QString sql, int generation_hash,
                           int params_hash) const {
  std::string cache_key = "query_cache:" + sql.toStdString() +
                          ":gen=" + std::to_string(generation_hash) +
                          ":params=" + std::to_string(params_hash);
  return cache_key;
}
