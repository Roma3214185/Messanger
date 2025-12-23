#pragma once

#include "interfaces/BaseQuery.h"
#include "interfaces/ICacheService.h"
#include "interfaces/ISqlExecutor.h"
#include "SqlBuilder.h"

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

void bindToQuery(std::unique_ptr<IQuery>& query, const QList<QVariant>& values) {
  if(!query) return;
  for (int i = 0; i < values.size(); ++i) query->bind(values[i]);
}

}  // namespace

template <typename T>
inline QVariant GenericRepository::toVariant(const Field& f, const T& entity) const {
  std::any val = f.get(&entity);

  if (f.type == typeid(long long))
    return QVariant::fromValue(std::any_cast<long long>(val));
  if (f.type == typeid(std::string))
    return QString::fromStdString(std::any_cast<std::string>(val));
  if (f.type == typeid(bool))
    return QVariant::fromValue(static_cast<int>(std::any_cast<bool>(val)));
  if (f.type == typeid(QDateTime)) {
    QDateTime dt = std::any_cast<QDateTime>(val);

    if (!dt.isValid()) {
      LOG_WARN("in to variant was invalid datetimp");
      dt = QDateTime::currentDateTime();
    }

    return QVariant(dt.toSecsSinceEpoch());
  }

  LOG_ERROR("Invalid type in toVariant for entity {}", nlohmann::json(entity).dump());
  return {};
}

template <typename T>
bool GenericRepository::save(const T& entity) {
  PROFILE_SCOPE("[repository] Save");

  const auto& meta = Reflection<T>::meta();
  const Field* idField = meta.find("id");
  if(idField) {
    long long entity_id = toVariant(*idField, entity).toLongLong();
    LOG_INFO("In entity id is {}", entity_id);
    if(entity_id <= 0) {
      LOG_ERROR("Failed to save {}, id {} is invalid", entity_id, nlohmann::json(entity).dump());
      return false;
    }
  }

  SqlBuilder<T> builder;
  SqlStatement smt_entity = builder.buildInsert(meta, entity);
  LOG_INFO("Builded sql {}", smt_entity.query.toStdString());
  database_.transaction();

  SqlStatement smt_outbox;
  smt_outbox.query = "INSERT INTO outbox (table_trigered, payload, processed) "
                     "VALUES (?, ?, 0)";

  std::string entity_json = nlohmann::json(entity).dump();

  smt_outbox.values =  QList<QVariant>{ QVariant(meta.table_name), QVariant(QString::fromStdString(entity_json)) };
  auto main_query = database_.prepare(smt_entity.query);
  auto outbox_query = database_.prepare( smt_outbox.query);

  if(!main_query || !outbox_query) {
    database_.rollback();
    LOG_INFO("Failed to prepare");
    return false;
  }

  LOG_INFO("Prepare succed");

  bindToQuery(main_query, smt_entity.values);
  bindToQuery(outbox_query, smt_outbox.values);

  if (!main_query->exec() || !outbox_query->exec()) {
    LOG_ERROR("[repository] Save failed for entity: {}", entity_json);
    database_.rollback();
    return false;
  }

  LOG_INFO("Save succeed for json: {}", entity_json);

  database_.commit();

  cache_.set(makeKey<T>(entity), entity_json);
  cache_.incr(std::string("table_generation:") + meta.table_name);
  return true;
}

template <typename T>
bool GenericRepository::save(std::vector<T>& entity) {
  bool res = true;
  for(auto el: entity) { //TODO: implement bathcer save
    res |= save(el);
  }
  return res;
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
  LOG_INFO("Id in dindOne {}", entity_id);
  auto query = QueryFactory::createSelect<T>(executor_, cache_);
  query->where("id", entity_id).limit(1);
  auto res = query->execute();
  return res.empty() ? std::nullopt : std::make_optional(res.front());
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

  //auto query = database_.prepare(sql);
  if(!executor_->execute(sql, {entity_id})) {
    // LOG_ERROR("[repository] SQL error on '{}': {}", meta.table_name,
    //           query.lastError().text().toStdString());
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
  LOG_INFO("findByField {}", field);
  auto query = QueryFactory::createSelect<T>(executor_, cache_);
  query->where(field, value);
  return query->execute();
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
