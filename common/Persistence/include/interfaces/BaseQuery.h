#ifndef BASEQUERY_H
#define BASEQUERY_H

#include <vector>
#include <cstdint>

#include "Meta.h"
#include "Query.h"
#include "threadpool.h"
#include "interfaces/ICacheService.h"
#include "interfaces/ISqlExecutor.h"

enum class Operator : std::uint8_t { Equal, Less, More, MoreEqual, NotEqual, LessEqual };

template <typename T>
class BaseQuery {
 protected:
  ISqlExecutor*            executor_;
  std::vector<QString>     involved_tables_;
  inline static ThreadPool pool{4};
  QStringList              filters_;
  QVector<QVariant>        values_;
  QString                  limit_clause_;
  QString                  join_clause_;
  QString                  table_name_;

 public:
  explicit BaseQuery(ISqlExecutor* executor) : executor_(executor) {
    table_name_ = QString::fromStdString(Reflection<T>::meta().table_name); //todo: remove from here table_name_ = (...)
    involved_tables_.push_back(table_name_);
  }

  virtual std::vector<T> execute() const = 0;

  BaseQuery& from(const std::string& table_name) & {
    table_name_ = QString::fromStdString(table_name);
    involved_tables_.push_back(table_name_);
    return *this;
  }

  BaseQuery& where(const std::string& field, const std::string& value) & {
    return where(field, QString::fromStdString(value));
  }

  BaseQuery& where(const std::string& field, const QVariant& value) & {
    filters_.push_back(QString("%1 = ?").arg(QString::fromStdString(field)));
    values_.push_back(value);
    return *this;
  }

  BaseQuery& where(const std::string& field, const Operator& op, const std::string& value) & {
    return where(field, op, QString::fromStdString(value));
  }

  BaseQuery& where(const std::string& field, const Operator& op, const QVariant& value) & {
    filters_.push_back(QString("%1 %2 ?").arg(QString::fromStdString(field))
                           .arg(QString::fromStdString(operator_to_sql_.at(op))));
    values_.push_back(value);
    return *this;
  }

  BaseQuery& limit(int n) & {
    limit_clause_ = QString("LIMIT %1").arg(n);
    return *this;
  }

  BaseQuery& join(const std::string& table,
                  const std::string& first,
                  const std::string& second) & {
    join_clause_ += QString(" JOIN %1 ON %2 = %3")
      .arg(QString::fromStdString(table))
        .arg(QString::fromStdString(first))
        .arg(QString::fromStdString(second));

    involved_tables_.push_back(QString::fromStdString(table));
    return *this;
  }
  private:
    const std::unordered_map<Operator, std::string> operator_to_sql_ {
      {Operator::Equal,      "="},
      {Operator::Less,       "<"},
      {Operator::More,       ">"},
      {Operator::MoreEqual,  ">="},
      {Operator::NotEqual,   "!="},
      {Operator::LessEqual,  "<="},
    };
};

template <typename T>
struct SelectQuery;

template <typename T>
struct DeleteQuery;

class QueryFactory {
 public:
  template <typename T>
  static std::unique_ptr<SelectQuery<T>> createSelect(ISqlExecutor*  executor,
                                                      ICacheService& cache) {
    return std::make_unique<SelectQuery<T>>(executor, cache);
  }

  template <typename T>
  static std::unique_ptr<DeleteQuery<T>> createDelete(ISqlExecutor*  executor,
                                                      ICacheService& cache) {
    return std::make_unique<DeleteQuery<T>>(executor, cache);
  }
};

#endif  // BASEQUERY_H
