#ifndef BACKEND_GENERICREPOSITORY_QUERY_H_
#define BACKEND_GENERICREPOSITORY_QUERY_H_

#include <QtSql/QSqlDatabase>
#include <string>
#include <unordered_map>
#include <vector>

#include "GenericRepository.h"
#include "Meta.h"
#include "RedisCache/RedisCache.h"
#include "SQLiteDataBase.h"
#include "ThreadPool.h"

struct Meta;

template <typename T>
struct Reflection;

template <typename T>
class Query {
  inline static ThreadPool pool{4};
  IDataBase& db;

 public:
  explicit Query(IDataBase& db) : db(db) {
    table_name_ = Reflection<T>::meta().table_name;
    involved_tables_.push_back(table_name_);
  }

  Query& filter(const std::string& field, const QVariant& value) {
    filters_.push_back(QString("%1 = ?").arg(QString::fromStdString(field)));
    values_.push_back(value);
    return *this;
  }

  Query& filter(const std::string& field, const std::string& op,
                const QVariant& value) {
    filters_.push_back(QString("%1 %2 ?")
                           .arg(QString::fromStdString(field))
                           .arg(QString::fromStdString(op)));
    values_.push_back(value);
    return *this;
  }

  Query& orderBy(const std::string& field,
                 const std::string& direction = "ASC") {
    order_ = QString("ORDER BY %1 %2")
                 .arg(QString::fromStdString(field))
                 .arg(QString::fromStdString(direction));
    return *this;
  }

  Query& limit(int n) {
    limit_clause_ = QString("LIMIT %1").arg(n);
    return *this;
  }

  std::vector<T> execute() const {
    QString sql = buildSelectQuery();
    auto generations = getGenerations();
    std::size_t generation_hash = hashGenerations(generations);
    std::size_t params_hash = hashParams(values_);

    std::string cache_key = createCacheKey(sql, generation_hash, params_hash);

    if (auto cached = cache_.get<std::vector<T>>(cache_key)) {
      LOG_INFO("[QueryCache] HIT for key '{}'", cache_key);
      return *cached;
    }

    LOG_INFO("[QueryCache] NOT HITTED for key '{}'", cache_key);

    QSqlDatabase thread_db = db.getThreadDatabase();
    QSqlQuery query(thread_db);
    if (!query.prepare(sql)) {
      qWarning() << "Prepare failed:" << query.lastError().text();
        return {};
    } else {
      LOG_INFO("Preparing success");
    }

    for (int i = 0; i < values_.size(); ++i) {
      query.bindValue(i, values_[i]);
    }

    if (!query.exec()) {
      LOG_ERROR("Query error: {}", query.lastError().text().toStdString());
      return {};
    }

    auto results = std::vector<T>{};
    auto meta = Reflection<T>::meta();

    while (query.next()) {
      T entity = buildEntity(query, meta);
      results.push_back(entity);
    }

    cache_.saveEntities(results, table_name_);

    LOG_INFO("Result size is '{}' is setted in cashe for key '{}'",
             results.size(), cache_key);
    cache_.set(cache_key, results, std::chrono::hours(24));
    return results;
  }

  std::future<std::vector<T>> executeAsync() const {
    return pool.enqueue([this]() { return execute(); });
  }

  std::future<std::vector<T>> executeWithoutCacheAsync() const {
    return pool.enqueue([this]() {
      return executeWithoutCache();  // DB initialized in this thread
    });
  }

  std::vector<T> executeWithoutCache() const {
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

  bool deleteAll() const {
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

 private:
  void saveEntityInCache(
      const T& entity, std::chrono::hours ttl = std::chrono::hours(24)) const {
    std::string entityKey = buildEntityKey(entity);
    cache_.set(entityKey, entity, ttl);
  }

  T buildEntity(QSqlQuery& query, const Meta& meta) const {
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

  int getEntityId(const T& entity) const { return entity.id; }

  QString buildSelectQuery() const {
    QString sql =
        QString("SELECT * FROM %1").arg(QString::fromStdString(table_name_));
    if (!filters_.empty()) sql += " WHERE " + filters_.join(" AND ");
    if (!order_.isEmpty()) sql += " " + order_;
    if (!limit_clause_.isEmpty()) sql += " " + limit_clause_;
    return sql;
  }

  auto getGenerations() const {
    std::unordered_map<std::string, long long> generations;
    for (const auto& table : involved_tables_) {
      std::string key = "table_generation:" + table;
      auto val = cache_.get<std::string>(key);
      generations[table] = val ? std::stoll(*val) : 0;
    }
    return generations;
  }

  std::size_t hashGenerations(
      const std::unordered_map<std::string, long long>& generations) const {
    std::size_t generation_hash = 0;
    for (const auto& [table, gen] : generations)
      generation_hash ^= std::hash<std::string>{}(table + std::to_string(gen));
    return generation_hash;
  }

  std::size_t hashParams(QVector<QVariant>) const {
    std::size_t params_hash = 0;
    for (const auto& v : values_)
      params_hash ^= std::hash<std::string>{}(v.toString().toStdString());
    return params_hash;
  }

  std::string buildEntityKey(const T& entity) const {
    return "entity_cache:" + table_name_ + ":" +
           std::to_string(getEntityId(entity));
  }

  std::string createCacheKey(QString sql, int generation_hash,
                             int params_hash) const {
    std::string cache_key = "query_cache:" + sql.toStdString() +
                            ":gen=" + std::to_string(generation_hash) +
                            ":params=" + std::to_string(params_hash);
    return cache_key;
  }

 private:
  RedisCache& cache_ = RedisCache::instance();
  std::vector<std::string> involved_tables_;
  QStringList filters_;
  QVector<QVariant> values_;
  QString order_;
  QString limit_clause_;
  std::string table_name_;
};

#endif  // BACKEND_GENERICREPOSITORY_QUERY_H_"
