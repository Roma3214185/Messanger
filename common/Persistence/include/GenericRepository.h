#ifndef BACKEND_GENERICREPOSITORY_GENERICREPOSITORY_H_
#define BACKEND_GENERICREPOSITORY_GENERICREPOSITORY_H_

#include <vector>

#include "CacheKeyGenerator.h"
#include "Meta.h"
#include "OutboxWorker.h"
#include "SqlBuilder.h"
#include "interfaces/IEntityBuilder.h"
#include "metaentity/EntityConcept.h"

template <EntityJson T>
using ResultList = std::vector<T>;
template <EntityJson T>
using FutureResultList = std::future<std::vector<T>>;

class ISqlExecutor;
class ICacheService;
class IThreadPool;
// class CacheKeyGenerator;

class GenericRepository {
  ISqlExecutor *executor_;
  ICacheService &cache_;
  IThreadPool *pool_;
  SqlBuilder builder_;
  IOutboxWorker *outbox_worker_;
  CacheKeyGenerator cache_kay_generator_;

 public:
  GenericRepository(ISqlExecutor *executor, ICacheService &cache, IThreadPool *pool_ = nullptr,
                    IOutboxWorker *outbox_worker = nullptr);

  ICacheService &getCache() { return cache_; }

  ISqlExecutor *getExecutor() { return executor_; }
  void clearCache();

  template <EntityJson T>
  bool save(const T &entity);

  template <EntityJson T>
  void saveAsync(T &entity);

  template <EntityJson T>
  std::optional<T> findOne(long long entity_id);

  template <EntityJson T>
  bool deleteById(long long entity_id);

  template <EntityJson T>
  bool deleteEntity(const T &entity);  // todo : update outbox

  template <EntityJson T>
  std::vector<T> findByField(const std::string &field, const std::string &value);

  template <EntityJson T>
  std::vector<T> findByField(const std::string &field, const QVariant &value);

  template <EntityJson T>
  T buildEntity(QSqlQuery &query, BuilderType type = BuilderType::Generic) const;

 private:
  template <EntityJson T>
  std::string makeKey(const T &entity) const;

  template <EntityJson T>
  [[nodiscard]] std::string makeKey(long long entity_id) const;
};

#include "GenericRepository.inl"

#endif  // BACKEND_GENERICREPOSITORY_GENERICREPOSITORY_H_
