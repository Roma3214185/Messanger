#ifndef BACKEND_GENERICREPOSITORY_QUERY_H_
#define BACKEND_GENERICREPOSITORY_QUERY_H_

#include <string>
#include <vector>

#include "interfaces/BaseQuery.h"
#include "interfaces/ICacheService.h"

struct Meta;

template <typename T>
struct Reflection;

template <typename T>
struct BaseQuery;

template <typename T>
class SelectQuery : public BaseQuery<T> {
  ICacheService& cache_;
  QString order_;
 public:
  SelectQuery(ISqlExecutor& executor, ICacheService& cashe);

  SelectQuery& orderBy(const std::string& field,
                 const std::string& direction = "ASC");
  std::vector<T> execute() const override;

  std::future<std::vector<T>> executeAsync() const;
  std::future<std::vector<T>> executeWithoutCacheAsync() const;
  std::vector<T> executeWithoutCache() const;

 protected:
  std::string createCacheKey(QString sql, int generation_hash,
                             int params_hash) const;
 private:
  std::vector<T> buildResults(QSqlQuery& query) const;
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
  std::string computeCacheKey(const QString& sql) const;
  std::optional<std::vector<T>> tryLoadFromCache(const std::string& key) const;
  QSqlQuery runDatabaseQuery(const QString& sql) const;
  void updateCache(const std::string& key, const std::vector<T>& results) const;
};

#include "Query.inl"

#endif  // BACKEND_GENERICREPOSITORY_QUERY_H_"
