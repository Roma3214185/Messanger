#ifndef QUERYFACTORY_H
#define QUERYFACTORY_H

#include "metaentity/EntityConcept.h"
#include "query/DeleteQuery.h"
#include "query/SelectQuery.h"

// template <EntityJson T>
// struct SelectQuery;

// template <EntityJson T>
// struct DeleteQuery;

class QueryFactory {
public:
  template <EntityJson T>
  static std::unique_ptr<SelectQuery<T>> createSelect(ISqlExecutor *executor,
                                                      ICacheService &cache) {
    return std::make_unique<SelectQuery<T>>(executor, cache);
  }

  template <EntityJson T>
  static std::unique_ptr<DeleteQuery<T>> createDelete(ISqlExecutor *executor,
                                                      ICacheService &cache) {
    return std::make_unique<DeleteQuery<T>>(executor, cache);
  }

  template <EntityJson T> // todo: another factory: QueryResultFactory
  static SelectResult<T> &getSelectResult(QueryResult<T> &value) {
    return expect_type<T, SelectResult<T>>(value);
  }

  template <EntityJson T>
  static DeleteResult<T> &getDeleteResult(QueryResult<T> &value) {
    return expect_type<T, DeleteResult<T>>(value);
  }

private:
  template <EntityJson T, typename VariantResult>
  requires VariantResultInQueryResult<VariantResult, QueryResult<T>>
  static VariantResult &expect_type(QueryResult<T> &value) {
    if (auto *p = std::get_if<VariantResult>(&value))
      return *p;
    throw std::runtime_error("Unexpected query result type");
  }
};

#endif // QUERYFACTORY_H
