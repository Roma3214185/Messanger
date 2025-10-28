#ifndef BACKEND_GENERICREPOSITORY_GENERICREPOSITORY_H_
#define BACKEND_GENERICREPOSITORY_GENERICREPOSITORY_H_

#include <functional>
#include <vector>
#include <unordered_map>
#include <utility>
#include <string>

#include <QtSql/qsqlquery.h>
#include <QDateTime>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>

#include "Debug_profiling.h"
#include "IEntityBuilder.h"
#include "MessageService/Headers/Message.h"
#include "Meta.h"
#include "Query.h"
#include "RedisCache/RedisCache.h"
#include "SQLiteDataBase.h"
#include "ThreadPool.h"

template <typename T>
using ResultList = std::vector<T>;
template <typename T>
using FutureResultList = std::future<std::vector<T>>;

class GenericRepository {
  IDataBase& database_;
  RedisCache& cache_ = RedisCache::instance();
  ThreadPool* pool_;
  std::unordered_map<std::string, QSqlQuery>
      stmt_cache_;

 public:
  explicit GenericRepository(IDataBase& database, ThreadPool* pool = nullptr)
      : database_(database), pool_(pool) {}

  QSqlDatabase& getThreadDatabase() { return database_.getThreadDatabase(); }

  void clearCache() { cache_.clearCache(); }

  template <typename T>
  void save(T& entity) {
    PROFILE_SCOPE("[repository] Save");
    auto meta = Reflection<T>::meta();
    LOG_INFO("Save in database_: '{}'", meta.table_name);

    const Field* idField = meta.find("id");
    if (!idField) {
      LOG_ERROR("[save] Missing id field in meta");
      throw std::runtime_error("Missing 'id' field in meta");
    }

    auto id = std::any_cast<long long>(idField->get(&entity));
    LOG_INFO("[repository] [save] id is '{}'", id);
    bool isInsert = (id == 0);

    auto values = QList<QVariant>{};
    auto [columns, placeholders] = buildInsertParts(meta, entity, values);

    QSqlDatabase conn = database_.getThreadDatabase();
    std::string stmtKey =
        meta.table_name + std::string(":save:") + std::to_string(columns.size());
    QString sql = QString("INSERT OR REPLACE INTO %1 (%2) VALUES (%3)")
                      .arg(QString::fromStdString(meta.table_name))
                      .arg(columns.join(", "))
                      .arg(placeholders.join(", "));

    // auto& query = getPreparedQuery(stmtKey, sql);
    QSqlQuery query(conn);
    query.prepare(sql);
    LOG_INFO("[repository] [save] values to insert size is '{}'",
             values.size());

    for (int i = 0; i < values.size(); ++i) query.bindValue(i, values[i]);

    if (!query.exec()) {
      LOG_ERROR("[repository] Save failed: '{}'",
                query.lastError().text().toStdString());
      throw std::runtime_error(
          ("Save failed: " + query.lastError().text()).toStdString());
    }

    LOG_INFO("[repository] Save successed");

    if (isInsert) {
      QVariant newId = query.lastInsertId();
      if (newId.isValid()) {
        LOG_INFO("id is valid: '{}'", newId.toLongLong());
        idField->set(&entity, newId.toLongLong());
      } else {
        LOG_ERROR("[repository] id isn't valid:");
      }
    }

    cache_.remove(makeKey<T>(id));
    cache_.incr(std::string("table_generation:") + meta.table_name);
  }

  template <typename T>
  void save(std::vector<T>& entities) {
    if (entities.empty()) return;

    PROFILE_SCOPE("[repository] Save batch");
    auto meta = Reflection<T>::meta();
    LOG_INFO("Save batch in database_: '{}', count: {}", meta.table_name,
             entities.size());

    const Field* idField = meta.find("id");
    if (!idField) {
      LOG_ERROR("[save] Missing id field in meta");
      throw std::runtime_error("Missing 'id' field in meta");
    }

    QSqlDatabase& conn = database_.getThreadDatabase();

    // Prepare SQL for batch insert
    QStringList allColumns;
    std::vector<QList<QVariant>> allValues;

    for (auto& entity : entities) {
      auto values = QList<QVariant>{};
      auto [columns, placeholders] = buildInsertParts(meta, entity, values);

      if (allColumns.empty()) {
        allColumns = QStringList();
        for (const auto& c : columns) allColumns << c;
      }

      allValues.push_back(values);
    }

    QStringList placeholdersList;
    for (size_t i = 0; i < allValues.size(); ++i) {
      QStringList ph;
      for (int j = 0; j < allValues[i].size(); ++j) ph << "?";
      placeholdersList << "(" + ph.join(", ") + ")";
    }

    QString sql = QString("INSERT OR REPLACE INTO %1 (%2) VALUES %3")
                      .arg(QString::fromStdString(meta.table_name))
                      .arg(allColumns.join(", "))
                      .arg(placeholdersList.join(", "));

    QSqlQuery query(conn);
    query.prepare(sql);

    // Bind all values
    int paramIndex = 0;
    for (auto& values : allValues) {
      for (auto& v : values) {
        query.bindValue(paramIndex++, v);
      }
    }

    if (!query.exec()) {
      LOG_ERROR("[repository] Save batch failed: '{}'",
                query.lastError().text().toStdString());
      throw std::runtime_error(
          ("Save batch failed: " + query.lastError().text()).toStdString());
    }

    LOG_INFO("[repository] Save batch successed, {} rows", entities.size());

    // Update IDs for inserted entities
    QVariant lastId = query.lastInsertId();
    for (auto& entity : entities) {
      if (lastId.isValid()) {
        idField->set(
            &entity,
            lastId.toLongLong());  // simple decrement to assign unique IDs
      }
      cache_.remove(makeKey<T>(
          std::any_cast<long long>(idField->get(&entity))));
    }

    cache_.incr(std::string("table_generation:") + meta.table_name);
  }

  template <typename T>
  void saveAsync(T& entity) {
    if (!pool_) {
      LOG_WARN("Pool isn't initialized");
      save(entity);
    } else {
      LOG_INFO("Start save async");
      pool_->enqueue([this, entity]() { return this->save<T>(entity); });
    }
  }

  template <typename T>
  std::future<std::optional<T>> findOneAsync(long long entity_id) {
    if (!pool_) {
      LOG_WARN("Pool isn't initialized");
      return std::async(std::launch::deferred,
                        [this, entity_id]() { return this->findOne<T>(entity_id); });
    } else {
      LOG_INFO("Start save async");
      return pool_->enqueue([this, entity_id]() { return this->findOne<T>(entity_id); });
    }
  }

  template <typename T>
  std::future<std::optional<T>> findOneWithOutCacheAsync(long long entity_id) {
    if (!pool_) {
      LOG_WARN("Pool isn't initialized");
      return std::async(std::launch::async, [this, entity_id]() {
        return this->findOneWithOutCache<T>(entity_id);
      });
    } else {
      return pool_->enqueue(
          [this, entity_id]() { return this->findOneWithOutCache<T>(entity_id); });
    }
  }

  template <typename T>
  std::optional<T> findOne(long long entity_id) {
    PROFILE_SCOPE("[repository] FindOne");
    const std::string key = makeKey<T>(entity_id);
    if (auto cache_d = cache_.get<T>(key)) {
      LOG_INFO("[repository] [cache_ HIT] key = '{}'", key);
      return cache_d;
    }
    LOG_INFO("[repository] cashe not hit key = '{}'", key);

    QSqlDatabase threaddatabase_ = database_.getThreadDatabase();
    auto meta = Reflection<T>::meta();

    QString sql = QString("SELECT * FROM %1 WHERE id = ?").arg(meta.table_name);
    std::string stmtKey = std::string(meta.table_name) + ":findOne";

    auto& query = getPreparedQuery(stmtKey, sql);

    query.bindValue(0, entity_id);

    if (!query.exec()) {
      LOG_ERROR("[repository] SQL error on '{}': {}", meta.table_name,
                query.lastError().text().toStdString());
      return std::nullopt;
    }

    if (!query.next()) {
      LOG_WARN("[repository] no rows found in '{}'", meta.table_name);
      return std::nullopt;
    }

    T entity = buildEntity<T>(query);
    cache_.set(key, entity);
    return entity;
  }

  QSqlQuery& getPreparedQuery(const std::string& stmtKey, const QString& sql) {
    QSqlDatabase threaddatabase_ = database_.getThreadDatabase();

    auto it = stmt_cache_.find(stmtKey);
    if (it == stmt_cache_.end()) {
      QSqlQuery query(threaddatabase_);
      query.prepare(sql);
      auto [insertIt, _] = stmt_cache_.emplace(stmtKey, std::move(query));
      return insertIt->second;
    } else {
      it->second.finish();
      it->second.clear();
      return it->second;
    }
  }

  template <typename T>
  std::optional<T> findOneWithOutCache(long long id) {
    const std::string key = makeKey<T>(id);
    QSqlDatabase thread_database = database_.getThreadDatabase();
    auto meta = Reflection<T>::meta();

    std::string stmtKey = std::string(meta.table_name) + ":findOne";
    QString sql = QString("SELECT * FROM %1 WHERE id = ?").arg(meta.table_name);
    auto& query = getPreparedQuery(stmtKey, sql);

    query.bindValue(0, id);

    if (!query.exec()) {
      LOG_ERROR("[repository] SQL error on '{}': {}", meta.table_name,
                query.lastError().text().toStdString());
      return std::nullopt;
    }

    if (!query.next()) {
      LOG_WARN("[repository] no rows found in '{}'", meta.table_name);
      return std::nullopt;
    }

    return buildEntity<T>(query);
  }

  template <typename T>
  void deleteEntity(T& entity) {
    deleteById<T>(entity.id);
  }

  template <typename T>
  void deleteById(long long id) {
    PROFILE_SCOPE("[repository] DeleteById");
    auto meta = Reflection<T>::meta();
    QString sql = QString("DELETE FROM %1 WHERE id = ?")
                      .arg(QString::fromStdString(meta.table_name));

    std::string stmKey = meta.table_name + std::string(":deleteById");
    // auto& query = getPreparedQuery(stmKey, sql);
    QSqlQuery query(database_.getThreadDatabase());
    query.prepare(sql);

    query.bindValue(0, id);

    if (!query.exec()) {
      LOG_ERROR("[repository] Delete failed: '{}'",
                query.lastError().text().toStdString());
      throw std::runtime_error("Delete failed: " +
                               query.lastError().text().toStdString());
    }
    cache_.incr(std::string("table_generation:") + meta.table_name);
    cache_.remove(makeKey<T>(id));
  }

  template <typename T>
  void deleteBatch(const std::vector<T>& batch) {
    PROFILE_SCOPE("[repository] DeleteBatch");
    if (batch.empty()) return;

    auto meta = Reflection<T>::meta();
    QString table_name = QString::fromStdString(meta.table_name);

    std::vector<long long> ids;
    ids.reserve(batch.size());
    for (const auto& item : batch) ids.push_back(item.id);

    QString placeholders;
    for (size_t i = 0; i < ids.size(); ++i) {
      if (i > 0) placeholders += ",";
      placeholders += "?";
    }

    QString sql = QString("DELETE FROM %1 WHERE id IN (%2)")
                      .arg(table_name)
                      .arg(placeholders);

    std::string stmKey = meta.table_name + std::string(":deleteBatch");
    // auto& query = getPreparedQuery(stmKey, sql);
    QSqlQuery query(database_.getThreadDatabase());
    query.prepare(sql);

    for (size_t i = 0; i < ids.size(); ++i)
      query.bindValue(static_cast<int>(i), ids[i]);

    if (!query.exec()) {
      LOG_ERROR("[repository] Delete batch failed: '{}'",
                query.lastError().text().toStdString());
      throw std::runtime_error("Delete batch failed: " +
                               query.lastError().text().toStdString());
    }

    cache_.incr(std::string("table_generation:") + meta.table_name);
    for (auto id : ids)
      cache_.remove(makeKey<T>(id));
  }

  template <typename T>
  std::vector<T> findByField(const std::string& field,
                             const std::string& value) {
    return findByField<T>(field, QVariant(QString::fromStdString(value)));
  }

  template <typename T>
  std::vector<T> findByField(const std::string& field, const QVariant& value) {
    return this->query<T>().filter(field, value).execute();
  }

  template <typename T>
  bool exists(long long id) {
    PROFILE_SCOPE("Exist");
    const std::string key = makeKey<T>(id);
    if (cache_.exists(key)) {
      LOG_INFO("[repository] cache_ HIT in exist true, key '{}'", key);
      return true;
    }

    auto meta = Reflection<T>::meta();
    LOG_INFO("[repository] cache_ not HIT in database_ '{}', key '{}'",
             meta.table_name, key);
    QString sql = QString("SELECT COUNT(1) FROM %1 WHERE id = ?")
                      .arg(QString::fromStdString(meta.table_name));
    std::string stmKey = meta.table_name + std::string(":exists");
    auto& query = getPreparedQuery(stmKey, sql);
    query.bindValue(0, id);

    if (!query.exec() || !query.next()) {
      LOG_ERROR("Id '{}' doen't exist", id);
      return false;
    }
    bool found = query.value(0).toLongLong() > 0;
    if (found) cache_.set(key, true, std::chrono::minutes(5));
    return found;
  }

  template <typename T>
  void truncate() {
    LOG_INFO("[repository] Truncate database '{}'");
    auto meta = Reflection<T>::meta();
    QString sql =
        QString("DELETE FROM %1").arg(QString::fromStdString(meta.table_name));
    std::string stmKey = meta.table_name + std::string(":truncate");
    auto& query = getPreparedQuery(stmKey, sql);
    if (!query.exec())
      throw std::runtime_error(
          ("Truncate failed: " + query.lastError().text()).toStdString());
    cache_.clearPrefix(meta.table_name + ":");
  }

  template <typename T>
  Query<T> query() {
    return Query<T>(this->database_);
  }

  template <typename T>
  T buildEntity(QSqlQuery& query, BuilderType type = BuilderType::Fast) const {
    auto builder = makeBuilder<T>(type);
    return builder->build(query);
  }

 private:
  template <typename T>
  std::string makeKey(const T& entity) const {
    return makeKey(entity.id);
  }

  template <typename T>
  std::string makeKey(long long id) const {
    return "entity_cache_:" + std::string(Reflection<T>::meta().table_name) +
           ":" + std::to_string(id);
  }

  template <typename T>
  long long getId(const T& obj) const {
    auto meta = Reflection<T>::meta();
    if (auto f = meta.find("id")) return std::any_cast<long long>(f->get(&obj));
    return 0;
  }

  template <typename T>
  QStringList buildUpdateParts(const Meta& meta, const T& entity,
                               QList<QVariant>& values) {
    QStringList sets;
    for (const auto& f : meta.fields) {
      if (std::string(f.name) == "id" &&
          std::string(meta.table_name) != "messages_status")
        continue;
      sets << QString("%1 = ?").arg(f.name);
      values << toVariant(f, entity);
    }
    return sets;
  }

  template <typename T>
  std::pair<QStringList, QStringList> buildInsertParts(
      const Meta& meta, const T& entity, QList<QVariant>& values) {
    QStringList cols, ph;
    for (const auto& f : meta.fields) {
      if (std::string(f.name) == "id" &&
          std::string(meta.table_name) != "messages_status")
        continue;
      cols << f.name;
      ph << "?";
      values << toVariant<T>(f, entity);
    }
    return {cols, ph};
  }

  template <typename T>
  QVariant toVariant(const Field& f, const T& entity) const {
    std::any val = f.get(&entity);

    LOG_INFO("'{}' type: '{}' any type '{}'", f.name, f.type.name(),
             val.type().name());

    if (f.type == typeid(long long))
      return QVariant::fromValue(std::any_cast<long long>(val));
    if (f.type == typeid(std::string))
      return QString::fromStdString(std::any_cast<std::string>(val));
    if (f.type == typeid(QDateTime)) {
      QDateTime dt = std::any_cast<QDateTime>(val);

      if (!dt.isValid()) {
        LOG_WARN("in to variant was invalid datetimp");
        dt = QDateTime::currentDateTime();
      }

      return QVariant(dt.toSecsSinceEpoch());
    }
    return {};
  }
};

#endif  // BACKEND_GENERICREPOSITORY_GENERICREPOSITORY_H_
