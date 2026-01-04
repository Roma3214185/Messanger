#ifndef BACKEND_GENERICREPOSITORY_QUERY_H_
#define BACKEND_GENERICREPOSITORY_QUERY_H_

#include <string>
#include <vector>

#include "interfaces/IBaseQuery.h"
#include "interfaces/ICacheService.h"
#include "metaentity/ChatMemberMeta.h"
#include "metaentity/ChatMeta.h"
#include "metaentity/EntityConcept.h"
#include "metaentity/MessageMeta.h"
#include "metaentity/MessageStatusMeta.h"
#include "metaentity/PrivateChatMeta.h"
#include "metaentity/UserCredentialsMeta.h"
#include "metaentity/UserMeta.h"

struct Meta;

template <typename T>
struct Reflection;

enum class OrderDirection { ASC, DESC };

template <EntityJson T>
class SelectQuery : public IBaseQuery<T> {
  ICacheService& cache_;
  QString        order_;
  SqlBuilder<T>  builder_;

 public:
  SelectQuery(ISqlExecutor* executor, ICacheService& cache);
  SelectQuery&   orderBy(const std::string&    field,
                         const OrderDirection& direction = OrderDirection::ASC) &;
  QueryResult<T> execute() const override;

  // todo: extract from this class work with cache
 private:
  void    saveEntityInCache(const T& entity, std::chrono::hours ttl = std::chrono::hours(24)) const;
  int     getEntityId(const T& entity) const;  // todo: make concept requires there is field id
  [[nodiscard]] QString buildQuery() const override;
  auto    getGenerations() const;
  [[nodiscard]] std::optional<std::vector<T>> tryLoadFromCache(const std::string& key) const;
  void updateCache(const std::string& key, const std::vector<T>& results) const;
};

#include "SelectQuery.inl"

#endif  // BACKEND_GENERICREPOSITORY_QUERY_H_"
