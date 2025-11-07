#pragma once

template <typename T>
inline QVariant GenericRepository::toVariant(const Field& f, const T& entity) const {
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



#include "SqlBuilder.h"
#include "RedisCache/ICacheService.h"
#include "interfaces/ISqlExecutor.h"


template <typename T>
bool GenericRepository::save(T& entity) {
  PROFILE_SCOPE("[repository] Save");

  const auto& meta = Reflection<T>::meta();
  const Field* idField = meta.find("id");
  bool need_to_return_id = idField != nullptr;

  SqlBuilder<T> builder;
  SqlStatement stmt = builder.buildInsert(meta, entity, need_to_return_id);

  QSqlQuery query;
  if (!executor_.execute(stmt.query, stmt.values, query)) {
    LOG_ERROR("[repository] Save failed");
    return false;
  }

  if (need_to_return_id && !updateEntityIdFromQuery(entity, query, idField))
      return false;

  //TODO: signal entity added - to update cache;

  cache_.incr(std::string("table_generation:") + meta.table_name);
  return true;
}


















template <typename T>
bool GenericRepository::updateEntityIdFromQuery(T& entity, QSqlQuery& query, const Field* idField) {
  if (!query.next()) return false;
  QVariant id_variant = query.value(0);
  if (!id_variant.isValid()) {
    LOG_ERROR("[repository] id isn't valid");
    return false;
  }

  auto new_id = id_variant.toLongLong();
  idField->set(&entity, new_id);
  cache_.remove(makeKey<T>(new_id));
  return true;
}






























template <typename T>
void GenericRepository::save(std::vector<T>& entities) {
  // if (entities.empty()) return;

  // PROFILE_SCOPE("[repository] Save batch");
  // auto meta = Reflection<T>::meta();
  // LOG_INFO("Save batch in database_: '{}', count: {}", meta.table_name,
  //          entities.size());

  // const Field* idField = meta.find("id");
  // bool need_to_return_id = idField;
  // LOG_INFO("Need to return id: {}", need_to_return_id);
  // QSqlDatabase& conn = database_.getThreadDatabase();

  // QStringList allColumns;
  // std::vector<QList<QVariant>> allValues; // = getValues();

  // QStringList placeholdersList;
  // for (size_t i = 0; i < allValues.size(); ++i) {
  //   QStringList ph;
  //   for (int j = 0; j < allValues[i].size(); ++j) ph << "?";
  //   placeholdersList << "(" + ph.join(", ") + ")";
  // }

  // QString sql = QString("INSERT OR REPLACE INTO %1 (%2) VALUES %3 RETURNING id")
  //                   .arg(QString::fromStdString(meta.table_name))
  //                   .arg(allColumns.join(", "))
  //                   .arg(placeholdersList.join(", "));

  // QSqlQuery query(conn);
  // query.prepare(sql);

  // // Bind all values
  // int paramIndex = 0;
  // for (auto& values : allValues) {
  //   for (auto& v : values) {
  //     query.bindValue(paramIndex++, v);
  //   }
  // }

  // if (!query.exec()) {
  //   LOG_ERROR("[repository] Save batch failed: '{}'",
  //             query.lastError().text().toStdString());
  //   throw std::runtime_error(
  //       ("Save batch failed: " + query.lastError().text()).toStdString());
  // }

  // LOG_INFO("[repository] Save batch successed, {} rows", entities.size());
  // int i = 0;
  // while(query.next()){
  //   auto entity_id = static_cast<long long>(query.value(0).toLongLong());
  //   entities[i++].id = entity_id;
  //   cache_.remove(makeKey<T>(entity_id));
  // }

  // cache_.incr(std::string("table_generation:") + meta.table_name);
}

template <typename T>
void GenericRepository::saveAsync(T& entity) {
  if (!pool_) {
    LOG_WARN("Pool isn't initialized");
    save(entity);
  } else {
    LOG_INFO("Start save async");
    pool_->enqueue([this, entity]() { return this->save<T>(entity); });
  }
}

template <typename T>
inline std::future<std::optional<T>> GenericRepository::findOneAsync(long long entity_id) {
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
std::future<std::optional<T>> GenericRepository::findOneWithOutCacheAsync(long long entity_id) {
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
std::optional<T> GenericRepository::findOne(long long entity_id) {
  // PROFILE_SCOPE("[repository] FindOne");
  // const std::string key = makeKey<T>(entity_id);
  // if (auto cache_d = cache_.get<T>(key)) {
  //   LOG_INFO("[repository] [cache_ HIT] key = '{}'", key);
  //   return cache_d;
  // }
  // LOG_INFO("[repository] cashe not hit key = '{}'", key);

  // QSqlDatabase thread_database = database_.getThreadDatabase();
  // auto meta = Reflection<T>::meta();

  // QString sql = QString("SELECT * FROM %1 WHERE id = ?").arg(meta.table_name);
  // std::string stmtKey = std::string(meta.table_name) + ":findOne";

  // //auto& query = getPreparedQuery(stmtKey, sql);
  // QSqlQuery query(thread_database);
  // query.prepare(sql);

  // query.bindValue(0, entity_id);

  // if (!query.exec()) {
  //   LOG_ERROR("[repository] SQL error on '{}': {}", meta.table_name,
  //             query.lastError().text().toStdString());
  //   return std::nullopt;
  // }

  // if (!query.next()) {
  //   LOG_WARN("[repository] no rows found in '{}'", meta.table_name);
  //   return std::nullopt;
  // }

  // T entity = buildEntity<T>(query);
  // cache_.set(key, entity);
  // return entity;
}

template <typename T>
std::optional<T> GenericRepository::findOneWithOutCache(long long id) {
  // const std::string key = makeKey<T>(id);
  // QSqlDatabase thread_database = database_.getThreadDatabase();
  // auto meta = Reflection<T>::meta();

  // std::string stmtKey = std::string(meta.table_name) + ":findOne";
  // QString sql = QString("SELECT * FROM %1 WHERE id = ?").arg(meta.table_name);
  // auto& query = getPreparedQuery(stmtKey, sql);

  // query.bindValue(0, id);

  // if (!query.exec()) {
  //   LOG_ERROR("[repository] SQL error on '{}': {}", meta.table_name,
  //             query.lastError().text().toStdString());
  //   return std::nullopt;
  // }

  // if (!query.next()) {
  //   LOG_WARN("[repository] no rows found in '{}'", meta.table_name);
  //   return std::nullopt;
  // }

  // return buildEntity<T>(query);
}

template <typename T>
void GenericRepository::deleteEntity(T& entity) {
  deleteById<T>(entity.id);
}

template <typename T>
void GenericRepository::deleteById(long long id) {
  // PROFILE_SCOPE("[repository] DeleteById");
  // auto meta = Reflection<T>::meta();
  // QString sql = QString("DELETE FROM %1 WHERE id = ?")
  //                   .arg(QString::fromStdString(meta.table_name));

  // std::string stmKey = meta.table_name + std::string(":deleteById");
  // // auto& query = getPreparedQuery(stmKey, sql);
  // QSqlQuery query(database_.getThreadDatabase());
  // query.prepare(sql);

  // query.bindValue(0, id);

  // if (!query.exec()) {
  //   LOG_ERROR("[repository] Delete failed: '{}'",
  //             query.lastError().text().toStdString());
  //   throw std::runtime_error("Delete failed: " +
  //                            query.lastError().text().toStdString());
  // }
  // cache_.incr(std::string("table_generation:") + meta.table_name);
  // cache_.remove(makeKey<T>(id));
}

template <typename T>
void GenericRepository::deleteBatch(const std::vector<T>& batch) {
  // PROFILE_SCOPE("[repository] DeleteBatch");
  // if (batch.empty()) return;

  // auto meta = Reflection<T>::meta();
  // QString table_name = QString::fromStdString(meta.table_name);

  // std::vector<long long> ids;
  // ids.reserve(batch.size());
  // for (const auto& item : batch) ids.push_back(item.id);

  // QString placeholders;
  // for (size_t i = 0; i < ids.size(); ++i) {
  //   if (i > 0) placeholders += ",";
  //   placeholders += "?";
  // }

  // QString sql = QString("DELETE FROM %1 WHERE id IN (%2)")
  //                   .arg(table_name)
  //                   .arg(placeholders);

  // std::string stmKey = meta.table_name + std::string(":deleteBatch");
  // // auto& query = getPreparedQuery(stmKey, sql);
  // QSqlQuery query(database_.getThreadDatabase());
  // query.prepare(sql);

  // for (size_t i = 0; i < ids.size(); ++i)
  //   query.bindValue(static_cast<int>(i), ids[i]);

  // if (!query.exec()) {
  //   LOG_ERROR("[repository] Delete batch failed: '{}'",
  //             query.lastError().text().toStdString());
  //   throw std::runtime_error("Delete batch failed: " +
  //                            query.lastError().text().toStdString());
  // }

  // cache_.incr(std::string("table_generation:") + meta.table_name);
  // for (auto id : ids)
  //   cache_.remove(makeKey<T>(id));
}

template <typename T>
std::vector<T> GenericRepository::findByField(const std::string& field,
                           const std::string& value) {
  return findByField<T>(field, QVariant(QString::fromStdString(value)));
}

template <typename T>
std::vector<T> GenericRepository::findByField(const std::string& field, const QVariant& value) {
  return this->query<T>().filter(field, value).execute();
}

template <typename T>
bool GenericRepository::exists(long long id) {
  // PROFILE_SCOPE("Exist");
  // const std::string key = makeKey<T>(id);
  // if (cache_.exists(key)) {
  //   LOG_INFO("[repository] cache_ HIT in exist true, key '{}'", key);
  //   return true;
  // }

  // auto meta = Reflection<T>::meta();
  // LOG_INFO("[repository] cache_ not HIT in database_ '{}', key '{}'",
  //          meta.table_name, key);
  // QString sql = QString("SELECT COUNT(1) FROM %1 WHERE id = ?")
  //                   .arg(QString::fromStdString(meta.table_name));
  // std::string stmKey = meta.table_name + std::string(":exists");
  // auto& query = getPreparedQuery(stmKey, sql);
  // query.bindValue(0, id);

  // if (!query.exec() || !query.next()) {
  //   LOG_ERROR("Id '{}' doen't exist", id);
  //   return false;
  // }
  // bool found = query.value(0).toLongLong() > 0;
  // if (found) cache_.set(key, true, std::chrono::minutes(5));
  // return found;
}

template <typename T>
void GenericRepository::truncate() {
  // LOG_INFO("[repository] Truncate database '{}'");
  // auto meta = Reflection<T>::meta();
  // QString sql =
  //     QString("DELETE FROM %1").arg(QString::fromStdString(meta.table_name));
  // std::string stmKey = meta.table_name + std::string(":truncate");
  // auto& query = getPreparedQuery(stmKey, sql);
  // if (!query.exec())
  //   throw std::runtime_error(
  //       ("Truncate failed: " + query.lastError().text()).toStdString());
  // cache_.clearPrefix(meta.table_name + ":");
}

template <typename T>
Query<T> GenericRepository::query() {
  //return Query<T>(this->database_);
}

template <typename T>
T GenericRepository::buildEntity(QSqlQuery& query, BuilderType type) const {
  //auto builder = makeBuilder<T>(type);
  //return builder->build(query);
}

template <typename T>
std::string GenericRepository::makeKey(const T& entity) const {
  return makeKey(entity.id);
}

template <typename T>
std::string GenericRepository::makeKey(long long id) const {
  //return "entity_cache_:" + std::string(Reflection<T>::meta().table_name) +
  //       ":" + std::to_string(id);
}

template <typename T>
long long GenericRepository::getId(const T& obj) const {
  //auto meta = Reflection<T>::meta();
  //if (auto f = meta.find("id")) return std::any_cast<long long>(f->get(&obj));
  //return 0;
}

template <typename T>
QStringList GenericRepository::buildUpdateParts(const Meta& meta, const T& entity,
                             QList<QVariant>& values) {
  // QStringList sets;
  // for (const auto& f : meta.fields) {
  //   if (std::string(f.name) == "id" &&
  //       std::string(meta.table_name) != "messages_status" &&
  //       std::string(meta.table_name) != "chat_members" &&
  //       std::string(meta.table_name) != "credentials")
  //     continue;
  //   sets << QString("%1 = ?").arg(f.name);
  //   values << toVariant(f, entity);
  // }
  // return sets;
}

template <typename T>
inline std::pair<QStringList, QStringList> GenericRepository::buildInsertParts(
    const Meta& meta, const T& entity, QList<QVariant>& values) {
  // QStringList cols, ph;
  // for (const auto& f : meta.fields) {
  //   if (std::string(f.name) == "id" &&  //TODO(roma) make another field id for this tables
  //       std::string(meta.table_name) != "messages_status" &&
  //       std::string(meta.table_name) != "chat_members" &&
  //       std::string(meta.table_name) != "credentials")
  //     continue;
  //   cols << f.name;
  //   ph << "?";
  //   values << toVariant<T>(f, entity);
  // }
  // return {cols, ph};
}
