#ifndef BACKEND_GENERICREPOSITORY_QUERY_H_
#define BACKEND_GENERICREPOSITORY_QUERY_H_

#include <string>
#include <vector>

#include "interfaces/BaseQuery.h"
#include "interfaces/ICacheService.h"

#include "metaentity/ChatMeta.h"
#include "metaentity/ChatMemberMeta.h"
#include "metaentity/MessageStatusMeta.h"
#include "metaentity/PrivateChatMeta.h"
#include "metaentity/UserMeta.h"
#include "metaentity/MessageMeta.h"
#include "metaentity/UserCredentialsMeta.h"

struct Meta;

template <typename T>
struct Reflection;

template <typename T>
struct BaseQuery;

enum class OrderDirection{ASC, DESC};

template <typename T>
class SelectQuery : public BaseQuery<T> {
  ICacheService& cache_;
  QString        order_;

 public:
  SelectQuery(ISqlExecutor* executor, ICacheService& cache);

  SelectQuery&   orderBy(const std::string& field, const OrderDirection& direction = OrderDirection::ASC) &;
  std::vector<T> execute() const override;

  std::future<std::vector<T>> executeAsync() const;
  std::future<std::vector<T>> executeWithoutCacheAsync() const;

 protected:
  [[nodiscard]] std::string createCacheKey(const QString& sql, int generation_hash, int params_hash) const;

 private:
  std::vector<T> buildResults(std::unique_ptr<IQuery>& query) const;
  void    saveEntityInCache(const T& entity, std::chrono::hours ttl = std::chrono::hours(24)) const;
  [[nodiscard]] T       buildEntity(std::unique_ptr<IQuery>& query, const Meta& meta) const;
  int     getEntityId(const T& entity) const;
  [[nodiscard]] QString buildSelectQuery() const; //todo: std::string_view return ??
  auto    getGenerations() const;
  [[nodiscard]] std::size_t hashGenerations(const std::unordered_map<std::string, std::string>& generations) const;
  [[nodiscard]] std::size_t hashParams(const QVector<QVariant>& values) const;
  [[nodiscard]] std::string buildEntityKey(const T& entity) const;
  [[nodiscard]] std::string computeCacheKey(const QString& sql) const;
  [[nodiscard]] std::optional<std::vector<T>> tryLoadFromCache(const std::string& key) const;
  void updateCache(const std::string& key, const std::vector<T>& results) const;
};

//todo: make pattern to update cache like read_impl

template <typename T>
class DeleteQuery : public BaseQuery<T> {  //todo: interface orderable
    ICacheService& cache_;
    QString        order_;

  public:
    DeleteQuery(ISqlExecutor* executor, ICacheService& cache);

    DeleteQuery&   orderBy(const std::string& field, const OrderDirection& direction = OrderDirection::ASC) &;
    std::vector<T> execute() const override;
    QString buildDeleteQuery() const;
};


#include "Query.inl"

#endif  // BACKEND_GENERICREPOSITORY_QUERY_H_"
