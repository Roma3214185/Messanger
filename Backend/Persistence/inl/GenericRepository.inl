#pragma once

#include "SqlBuilder.h"
#include "RedisCache/ICacheService.h"
#include "interfaces/ISqlExecutor.h"


namespace {

template <typename T>
bool needsIdReturn(const Field* idField, const T& entity) {
  if (!idField) return false;

  std::any idAny = idField->get(&entity);
  if (!idAny.has_value()) return false;

  if (idAny.type() == typeid(int))
    return std::any_cast<int>(idAny) == 0;
  if (idAny.type() == typeid(long long))
    return std::any_cast<long long>(idAny) == 0;
  if (idAny.type() == typeid(qulonglong))
    return std::any_cast<qulonglong>(idAny) == 0;

  return false;
}

}  // namespace

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

template <typename T>
bool GenericRepository::save(T& entity) {
  PROFILE_SCOPE("[repository] Save");

  const auto& meta = Reflection<T>::meta();
  const Field* idField = meta.find("id");
  bool need_to_return_id = needsIdReturn<T>(idField, entity);
  LOG_INFO("Need to return id {}", need_to_return_id);

  SqlBuilder<T> builder;
  SqlStatement stmt = builder.buildInsert(meta, entity, need_to_return_id);

  QSqlQuery query;
  if (!executeStatement(stmt, idField, entity, query, need_to_return_id)) {
    LOG_ERROR("[repository] Save batch failed: '{}'",
              query.lastError().text().toStdString());
    return false;
  }

  cache_.set(makeKey<T>(entity), entity);
  cache_.incr(std::string("table_generation:") + meta.table_name);
  return true;
}

template <typename T>
bool GenericRepository::save(std::vector<T>& entity) {
  bool res = true;
  for(auto el: entity) {
    res |= save(el);
  }
  return res;
}

template <typename T>
bool GenericRepository::executeStatement(const SqlStatement& stmt, const Field* idField, T& entity, QSqlQuery& query, bool need_to_return_id) {
  if (!need_to_return_id) {
    return executor_.execute(stmt.query, stmt.values, query);
  }

  auto returned_id = executor_.executeReturningId(stmt.query, stmt.values, query);
  if (!returned_id.has_value()) return false;

  assert(idField != nullptr);
  LOG_INFO("Returned id {}", *returned_id);
  idField->set(&entity, *returned_id);
  return true;
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
std::optional<T> GenericRepository::findOne(long long entity_id) {
  auto res = query<T>().filter("id", entity_id).execute();
  if(res.empty()) return std::nullopt;
  return res.front();
}

template <typename T>
void GenericRepository::deleteEntity(T& entity) {
  deleteById<T>(entity.id); //? TODO: not each entity has id field
}

template <typename T>
void GenericRepository::deleteBatch(std::vector<T>& batch) {
  qDebug() << "Not implemented";
}

template <typename T>
void GenericRepository::deleteById(long long entity_id) {
  PROFILE_SCOPE("[repository] DeleteById");
  auto meta = Reflection<T>::meta();
  QString sql = QString("DELETE FROM %1 WHERE id = ?")
                    .arg(QString::fromStdString(meta.table_name));

  std::string stmKey = meta.table_name + std::string(":deleteById");


  QSqlQuery query(database_.getThreadDatabase());
  if(!executor_.execute(sql, {entity_id}, query)) {
    LOG_ERROR("[repository] SQL error on '{}': {}", meta.table_name,
              query.lastError().text().toStdString());
    return;
  }

  cache_.incr(std::string("table_generation:") + meta.table_name);
  cache_.remove(makeKey<T>(entity_id));
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
Query<T> GenericRepository::query() {
  return Query<T>(this->database_);
}

template <typename T>
T GenericRepository::buildEntity(QSqlQuery& query, BuilderType type) const {
  auto builder = makeBuilder<T>(type);
  return builder->build(query);
}

template <typename T>
std::string GenericRepository::makeKey(const T& entity) const {
  EntityKey<T> keyBuilder;
  return "entity_cache:" + std::string(Reflection<T>::meta().table_name) +
         ":" + keyBuilder.get(entity);
}

template <typename T>
std::string GenericRepository::makeKey(long long id) const {
  return "entity_cache:" + std::string(Reflection<T>::meta().table_name) +
        ":" + std::to_string(id);
}

template <typename T>
long long GenericRepository::getId(const T& obj) const {
  auto meta = Reflection<T>::meta();
  if (auto f = meta.find("id")) return std::any_cast<long long>(f->get(&obj));
  return 0;
}
