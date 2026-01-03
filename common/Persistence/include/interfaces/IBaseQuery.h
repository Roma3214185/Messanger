#ifndef IBASEQUERY_H
#define IBASEQUERY_H

#include <vector>
#include <cstdint>

#include "Meta.h"
#include "threadpool.h"
#include "interfaces/ICacheService.h"
#include "interfaces/ISqlExecutor.h"
#include "metaentity/EntityConcept.h"
#include "query/QueryResult.h"
#include "Debug_profiling.h"

enum class Operator : std::uint8_t { Equal, Less, More, MoreEqual, NotEqual, LessEqual };

template <EntityJson T>
class IBaseQuery {
  protected:
    ISqlExecutor* executor_;

    std::vector<QString> involved_tables_;
    QStringList          filters_;
    QVector<QVariant>    values_;
    QString              limit_clause_;
    QString              join_clause_;
    QString              table_name_;

    inline static ThreadPool pool{ 4 };

  public:
    explicit IBaseQuery(ISqlExecutor* executor);

    IBaseQuery& from(const std::string& table_name) &;
    IBaseQuery& where(const std::string& field, const std::string& value) &;
    IBaseQuery& where(const std::string& field, const QVariant& value) &;
    IBaseQuery& where(const std::string& field, Operator op, const std::string& value) &;
    IBaseQuery& where(const std::string& field, Operator op, const QVariant& value) &;
    IBaseQuery& limit(int n) &;
    IBaseQuery& join(const std::string& table,
                     const std::string& first,
                     const std::string& second) &;

    [[nodiscard]]
    virtual QueryResult<T> execute() const = 0;

  protected:
    [[nodiscard]]
    virtual QString buildQuery() const = 0;

    virtual ~IBaseQuery() = default;
};

#include "IBaseQuery.inl"

#endif  // IBASEQUERY_H
