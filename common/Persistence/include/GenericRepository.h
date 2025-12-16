#ifndef BACKEND_GENERICREPOSITORY_GENERICREPOSITORY_H_
#define BACKEND_GENERICREPOSITORY_GENERICREPOSITORY_H_

#include <QtSql/qsqlquery.h>

#include <QDateTime>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Meta.h"
#include "Query.h"
#include "RedisCache.h"
#include "SQLiteDataBase.h"
#include "SqlBuilder.h"
#include "interfaces/IEntityBuilder.h"
#include "OutboxWorker.h"

#include "interfaces/entity.h"

template <typename T>
using ResultList = std::vector<T>;
template <typename T>
using FutureResultList = std::future<std::vector<T>>;

class ISqlExecutor;
class ICacheService;
class IThreadPool;
class IIdGenerator;

class GenericRepository {
  ISqlExecutor*  executor_;
  ICacheService& cache_;
  IThreadPool*    pool_;
  IDataBase&     database_;
  IIdGenerator* generator_;
  std::unique_ptr<OutboxWorker> outbox_worker_;

 public:
  GenericRepository(IDataBase&     database,
                    ISqlExecutor* executor,
                    ICacheService& cache,
                    IIdGenerator* generator,
                    IThreadPool*    pool_ = nullptr);
  ~GenericRepository();

  ICacheService& getCache() { return cache_; }

  IDataBase& getDatabase() { return database_; }
  ISqlExecutor* getExecutor() { return executor_; }
  void       clearCache();

  template <typename T>
  bool save(T& entity);

  template <typename T>
  bool save(std::vector<T>& entity);

  template <typename T>
  void saveAsync(T& entity);

  template <typename T>
  std::future<std::optional<T>> findOneAsync(long long entity_id);

  template <typename T>
  std::optional<T> findOne(long long entity_id);

  template <typename T>
  void deleteById(long long id);

  template <typename T>
  void deleteEntity(T& id);

  template <typename T>
  void deleteBatch(std::vector<T>& id);

  template <typename T>
  std::vector<T> findByField(const std::string& field, const std::string& value);

  template <typename T>
  std::vector<T> findByField(const std::string& field, const QVariant& value);

  template <typename T>
  T buildEntity(QSqlQuery& query, BuilderType type = BuilderType::Generic) const;

 private:
  template <typename T>
  std::string makeKey(const T& entity) const;

  template <typename T>
  std::string makeKey(long long id) const;

  template <typename T>
  long long getId(const T& obj) const;

  template <typename T>
  QVariant toVariant(const Field& f, const T& entity) const;
};

#include "GenericRepository.inl"

#endif  // BACKEND_GENERICREPOSITORY_GENERICREPOSITORY_H_
