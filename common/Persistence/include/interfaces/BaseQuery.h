#ifndef BASEQUERY_H
#define BASEQUERY_H

#include <vector>

#include "Meta.h"
#include "Query.h"
#include "ThreadPool.h"
#include "interfaces/ICacheService.h"
#include "interfaces/ISqlExecutor.h"

template <typename T>
class BaseQuery {
 protected:
  ISqlExecutor*            executor_;
  std::vector<std::string> involved_tables_;
  inline static ThreadPool pool{4};
  QStringList              filters_;
  QVector<QVariant>        values_;
  QString                  limit_clause_;
  std::string              join_clause_;
  std::string              table_name_;

 public:
  explicit BaseQuery(ISqlExecutor* executor) : executor_(executor) {
    table_name_ = Reflection<T>::meta().table_name;
    involved_tables_.push_back(table_name_);
  }

  virtual std::vector<T> execute() const = 0;

  BaseQuery& where(const std::string& field, const QVariant& value) {
    filters_.push_back(QString("%1 = ?").arg(QString::fromStdString(field)));
    values_.push_back(value);
    return *this;
  }
  BaseQuery& where(const std::string& field, const std::string& op, const QVariant& value) {
    filters_.push_back(
        QString("%1 %2 ?").arg(QString::fromStdString(field)).arg(QString::fromStdString(op)));
    values_.push_back(value);
    return *this;
  }

  BaseQuery& limit(int n) {
    limit_clause_ = QString("LIMIT %1").arg(n);
    return *this;
  }

  BaseQuery& join(const std::string& table,
                  const std::string& on)
  {
    join_clause_ += " JOIN " + table + " ON " + on;
    involved_tables_.push_back(table);
    return *this;
  }
};

template <typename T>
class SelectQuery;

class QueryFactory {
 public:
  template <typename T>
  static std::unique_ptr<SelectQuery<T>> createSelect(ISqlExecutor*  executor,
                                                      ICacheService& cache) {
    return std::make_unique<SelectQuery<T>>(executor, cache);
  }
};

#endif  // BASEQUERY_H
