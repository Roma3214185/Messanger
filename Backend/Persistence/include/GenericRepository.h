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
#include "interfaces/IEntityBuilder.h"
#include "MessageService/include/entities/Message.h"
#include "Meta.h"
#include "Query.h"
#include "RedisCache/RedisCache.h"
#include "SQLiteDataBase.h"
#include "ThreadPool.h"
#include "Persistence/include/SqlBuilder.h"

template <typename T>
using ResultList = std::vector<T>;
template <typename T>
using FutureResultList = std::future<std::vector<T>>;

class ISqlExecutor;
class ICacheService;

class GenericRepository {
  ISqlExecutor& executor_;
  ICacheService& cache_;
  ThreadPool* pool_;
  IDataBase& database_;

 public:
  GenericRepository(IDataBase& database, ISqlExecutor& executor,
                     ICacheService& cache, ThreadPool* pool_ = nullptr);

  IDataBase& getDatabase() { return database_; }
  void clearCache();

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
  std::vector<T> findByField(const std::string& field,
                             const std::string& value);

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

  template <typename T>
  bool executeStatement(const SqlStatement& stmt, const Field* idField, T& entity,
                        QSqlQuery& query, bool need_to_return_id);
};

#include "Persistence/inl/GenericRepository.inl"

#endif  // BACKEND_GENERICREPOSITORY_GENERICREPOSITORY_H_
