#pragma once

#include "Persistence/Query.h"
#include "Persistence/include/SqlExecutor.h"
#include <nlohmann/json.hpp>

template <typename T>
Query<T>::Query(IDataBase& db) : db(db) {
  table_name_ = Reflection<T>::meta().table_name;
  involved_tables_.push_back(table_name_);
}

template <typename T>
Query<T>& Query<T>::filter(const std::string& field, const QVariant& value) {
  filters_.push_back(QString("%1 = ?").arg(QString::fromStdString(field)));
  values_.push_back(value);
  return *this;
}

template <typename T>
Query<T>& Query<T>::filter(const std::string& field, const std::string& op,
              const QVariant& value) {
  filters_.push_back(QString("%1 %2 ?")
                         .arg(QString::fromStdString(field))
                         .arg(QString::fromStdString(op)));
  values_.push_back(value);
  return *this;
}

template <typename T>
Query<T>& Query<T>::orderBy(const std::string& field,
               const std::string& direction) {
  order_ = QString("ORDER BY %1 %2")
  .arg(QString::fromStdString(field))
      .arg(QString::fromStdString(direction));
  return *this;
}

template <typename T>
Query<T>& Query<T>::limit(int n) {
  limit_clause_ = QString("LIMIT %1").arg(n);
  return *this;
}

template <typename T>
std::vector<T> Query<T>::execute() const {
  QString sql = buildSelectQuery();
  auto generations = getGenerations();
  std::size_t generation_hash = hashGenerations(generations);
  std::size_t params_hash = hashParams(values_);

  std::string cache_key = createCacheKey(sql, generation_hash, params_hash);

  if (auto entity_json = cache_.get(cache_key)) {
    LOG_INFO("[QueryCache] HIT for key '{}'", cache_key);
    auto res = entity_json->template get<std::vector<T>>();
    LOG_INFO("[QueryCache] Parse successfull: '{}'", res.size());
    return res;
  }

  LOG_INFO("[QueryCache] NOT HITTED for key '{}'", cache_key);

  // QSqlDatabase thread_db = db.getThreadDatabase();
  // QSqlQuery query(thread_db);
  // if (!query.prepare(sql)) {
  //   qWarning() << "Prepare failed:" << query.lastError().text();
  //   return {};
  // } else {
  //   LOG_INFO("Preparing success");
  // }

  QSqlQuery query;
  SqlExecutor executer(db);
  if (!executer.execute(sql, values_, query)) {
    return {};
  }
  // for (int i = 0; i < values_.size(); ++i) {
  //   query.bindValue(i, values_[i]);
  // }

  // if (!query.exec()) {
  //   LOG_ERROR("Query error: {}", query.lastError().text().toStdString());
  //   return {};
  // }

  auto entities_jsons = std::vector<nlohmann::json>{};
  auto entities_keys = std::vector<std::string>{};
  auto results = std::vector<T>{};
  auto meta = Reflection<T>::meta();

  while (query.next()) {
    T entity = buildEntity(query, meta);
    std::string entityKey = buildEntityKey(entity);

    results.push_back(entity);
    entities_jsons.push_back(entity);
    entities_keys.push_back(entityKey);
    LOG_INFO("Select {}", entities_jsons.back().dump());
  }

  cache_.setPipelines(entities_keys, entities_jsons);
  cache_.set(cache_key, results, std::chrono::hours(24));
  return results;
}

template <typename T>
std::future<std::vector<T>> Query<T>::executeAsync() const {
  return pool.enqueue([this]() { return execute(); });
}

template <typename T>
std::future<std::vector<T>> Query<T>::executeWithoutCacheAsync() const {
  return pool.enqueue([this]() {
    return executeWithoutCache();  // DB initialized in this thread
  });
}

template <typename T>
std::vector<T> Query<T>::executeWithoutCache() const {
  QString sql = buildSelectQuery();
  QSqlDatabase threadDb =
      db.getThreadDatabase();  // safe: value, thread-local initialized in
      // current thread
  QSqlQuery query(threadDb);
  query.prepare(sql);
  for (int i = 0; i < values_.size(); ++i) query.bindValue(i, values_[i]);

  if (!query.exec()) {
    LOG_ERROR("Query error: {}", query.lastError().text().toStdString());
    return {};
  }

  std::vector<T> results;
  auto meta = Reflection<T>::meta();
  while (query.next()) {
    results.push_back(buildEntity(query, meta));
  }

  LOG_INFO("Result size is '{}'", results.size());
  return results;
}

template <typename T>
bool Query<T>::deleteAll() const {
  QString sql =
      QString("DELETE FROM %1").arg(QString::fromStdString(table_name_));
  if (!filters_.empty()) sql += " WHERE " + filters_.join(" AND ");

  QSqlQuery query(db);
  query.prepare(sql);
  for (int i = 0; i < values_.size(); ++i) query.bindValue(i, values_[i]);

  if (!query.exec()) {
    LOG_ERROR("Delete query failed: {}",
              query.lastError().text().toStdString());
    return false;
  }

  LOG_INFO("Deleted {} rows from {}", query.numRowsAffected(), table_name_);
  return true;
}

template <typename T>
void Query<T>::saveEntityInCache(
    const T& entity, std::chrono::hours ttl) const {
  std::string entityKey = buildEntityKey(entity);
  cache_.set(entityKey, entity, ttl);
}

template <typename T>
T Query<T>::buildEntity(QSqlQuery& query, const Meta& meta) const {
  T entity;
  for (const auto& f : meta.fields) {
    QVariant v = query.value(f.name);
    if (!v.isValid()) continue;

    std::any val;
    if (f.type == typeid(long long))
      val = v.toLongLong();
    else if (f.type == typeid(std::string))
      val = v.toString().toStdString();
    else if (f.type == typeid(QDateTime))
      val = v.toDateTime();
    f.set(&entity, val);
  }
  return entity;
}

template <typename T>
int Query<T>::getEntityId(const T& entity) const { return entity.id; }

template <typename T>
QString Query<T>::buildSelectQuery() const {
  QString sql =
      QString("SELECT * FROM %1").arg(QString::fromStdString(table_name_));
  if (!filters_.empty()) sql += " WHERE " + filters_.join(" AND ");
  if (!order_.isEmpty()) sql += " " + order_;
  if (!limit_clause_.isEmpty()) sql += " " + limit_clause_;
  return sql;
}

template <typename T>
auto Query<T>::getGenerations() const {
  std::unordered_map<std::string, long long> generations;
  for (const auto& table : involved_tables_) {
    std::string key = "table_generation:" + table;
    std::optional<nlohmann::json> json_ptr = cache_.get(key);
    generations[table] = json_ptr ? std::stoll(json_ptr->dump()) : 0;
  }
  return generations;
}

template <typename T>
std::size_t Query<T>::hashGenerations(
    const std::unordered_map<std::string, long long>& generations) const {
  std::size_t generation_hash = 0;
  for (const auto& [table, gen] : generations)
    generation_hash ^= std::hash<std::string>{}(table + std::to_string(gen));
  return generation_hash;
}

template <typename T>
std::size_t Query<T>::hashParams(QVector<QVariant>) const {
  std::size_t params_hash = 0;
  for (const auto& v : values_)
    params_hash ^= std::hash<std::string>{}(v.toString().toStdString());
  return params_hash;
}

template <typename T>
std::string Query<T>::buildEntityKey(const T& entity) const {
  return "entity_cache:" + table_name_ + ":" + EntityKey<T>::get(entity);
}

template <typename T>
std::string Query<T>::createCacheKey(QString sql, int generation_hash,
                           int params_hash) const {
  std::string cache_key = "query_cache:" + sql.toStdString() +
                          ":gen=" + std::to_string(generation_hash) +
                          ":params=" + std::to_string(params_hash);
  return cache_key;
}
