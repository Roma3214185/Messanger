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
 public:
  explicit Query(IDataBase& db);

  Query& filter(const std::string& field, const QVariant& value);

  Query& filter(const std::string& field, const std::string& op,
                const QVariant& value);

  Query& orderBy(const std::string& field,
                 const std::string& direction = "ASC");

  std::vector<T> execute() const;

  Query& limit(int n);

  std::future<std::vector<T>> executeAsync() const;

  std::future<std::vector<T>> executeWithoutCacheAsync() const;

  std::vector<T> executeWithoutCache() const;

  bool deleteAll() const;

 private:
  void saveEntityInCache(
       const T& entity, std::chrono::hours ttl = std::chrono::hours(24)) const;

  T buildEntity(QSqlQuery& query, const Meta& meta) const;

  int getEntityId(const T& entity) const;

  QString buildSelectQuery() const;

  auto getGenerations() const;

  std::size_t hashGenerations(
      const std::unordered_map<std::string, long long>& generations) const;

  std::size_t hashParams(QVector<QVariant>) const;

  std::string buildEntityKey(const T& entity) const;

  std::string createCacheKey(QString sql, int generation_hash,
                             int params_hash) const;

 private:
  RedisCache& cache_ = RedisCache::instance();
  std::vector<std::string> involved_tables_;
  inline static ThreadPool pool{4};
  IDataBase& db;
  QStringList filters_;
  QVector<QVariant> values_;
  QString order_;
  QString limit_clause_;
  std::string table_name_;
};

#include "Persistence/inl/Query.inl"

#endif  // BACKEND_GENERICREPOSITORY_QUERY_H_"
