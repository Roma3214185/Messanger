#ifndef DELETEQUERY_H
#define DELETEQUERY_H

#include "interfaces/IBaseQuery.h"
#include "metaentity/EntityConcept.h"

template <EntityJson T>
class DeleteQuery : public IBaseQuery<T> {
    ICacheService& cache_;

  public:
    DeleteQuery(ISqlExecutor* executor, ICacheService& cache);
    QueryResult<T> execute() const override;

  private:
    QString buildQuery() const override;
};

#include "DeleteQuery.inl"

#endif // DELETEQUERY_H
