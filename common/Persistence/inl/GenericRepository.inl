#ifndef INL_GENERIC_REPOSITORY
#define INL_GENERIC_REPOSITORY

#include "interfaces/IBaseQuery.h"
#include "interfaces/ICacheService.h"
#include "interfaces/ISqlExecutor.h"
#include "SqlBuilder.h"
#include "QueryFactory.h"
#include "GenericRepository.h"

template <EntityJson T>
bool GenericRepository::save(const T& entity) {
  std::string entity_json = nlohmann::json(entity).dump();

  if(!entity.checkInvariants()) { //todo: DBC_REQUIRE(entity.checkInvariants());
    LOG_ERROR("checkInvariants failed for entity {}", entity_json);
    return false;
  }

  const auto& meta = Reflection<T>::meta();

  SqlStatement smt_entity = builder_.buildInsert<T>(meta, entity);
  LOG_INFO("Builded sql {}", smt_entity.query.toStdString());
  if(auto result = executor_->execute(smt_entity.query, smt_entity.values); !result.query) {
    LOG_ERROR("Failed to save {}, reason - {}", entity_json, result.error);
    return false;
  }

  LOG_INFO("Save succeed for json: {}", entity_json);

  cache_.set(cache_kay_generator_.makeKey<T>(entity), entity_json, std::chrono::seconds{30});
  cache_.incr(std::string("table_generation:") + meta.table_name);
  return true;
}

// template <EntityJson T>
// bool GenericRepository::save(std::vector<T>& entities) {
//   bool res = true;
//   for(auto &entity: entities) { //TODO: implement bathcer save
//     res |= save(entity);
//   }
//   return res;
// }

template <EntityJson T>
void GenericRepository::saveAsync(T& entity) {
  if (!pool_) {
    LOG_WARN("Pool isn't initialized");
    save(entity);
  } else {
    LOG_INFO("Start save async");
    pool_->enqueue([this, entity]() { return this->save<T>(entity); });
  }
}

// template <EntityJson T>
// inline std::future<std::optional<T>> GenericRepository::findOneAsync(long long entity_id) {
//   if (!pool_) {
//     LOG_WARN("Pool isn't initialized");
//     return std::async(std::launch::deferred,
//                       [this, entity_id]() { return this->findOne<T>(entity_id); });
//   } else {
//     LOG_INFO("Start save async");
//     return pool_->enqueue([this, entity_id]() { return this->findOne<T>(entity_id); });
//   }
// }

template <EntityJson T>
std::optional<T> GenericRepository::findOne(long long entity_id) {
  LOG_INFO("Id in dindOne {}", entity_id);
  auto query = QueryFactory::createSelect<T>(executor_, cache_);
  query->where("id", entity_id).limit(1);
  auto res = query->execute();
  auto select_res = QueryFactory::getSelectResult(res);
  return select_res.result.empty() ? std::nullopt : std::make_optional(select_res.result.front());
}

template <EntityJson T>
bool GenericRepository::deleteEntity(const T& entity) {
  return deleteById<T>(entity.id); //? TODO: not each entity has id field
}

// template <EntityJson T>
// void GenericRepository::deleteBatch(std::vector<T>& batch) {
//   qDebug() << "Not implemented";
// }

template <EntityJson T>
bool GenericRepository::deleteById(long long entity_id) {
  PROFILE_SCOPE("[repository] DeleteById");
  auto meta = Reflection<T>::meta();
  QString sql = QString("DELETE FROM %1 WHERE id = ?")
                    .arg(QString::fromStdString(meta.table_name));

  if(auto result = executor_->execute(sql, {entity_id}); !result.query) {
    LOG_ERROR("Failed to delete by id {}, error {}", entity_id, result.error);
    return false;
  }

  // todo: std::string stmKey = meta.table_name + std::string(":deleteById");
  cache_.incr(std::string("table_generation:") + meta.table_name);
  cache_.remove(cache_kay_generator_.makeKey<T>(entity_id));
  return true;
}

template <EntityJson T>
std::vector<T> GenericRepository::findByField(const std::string& field,
                           const std::string& value) {
  return findByField<T>(field, QVariant(QString::fromStdString(value)));
}

template <EntityJson T>
std::vector<T> GenericRepository::findByField(const std::string& field, const QVariant& value) {
  LOG_INFO("findByField {}", field);
  auto query = QueryFactory::createSelect<T>(executor_, cache_);
  query->where(field, value);
  auto res = query->execute();
  auto select_res = QueryFactory::getSelectResult(res);
  return select_res.result;
}

template <EntityJson T>
T GenericRepository::buildEntity(QSqlQuery& query, BuilderType type) const {
  auto builder = makeBuilder<T>(type);
  return builder->build(query);
}

#endif
