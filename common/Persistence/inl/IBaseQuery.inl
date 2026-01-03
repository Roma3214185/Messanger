#include "interfaces/IBaseQuery.h"
#include "Debug_profiling.h"

namespace {

const static std::unordered_map<Operator, std::string> operator_to_sql = {
    {Operator::Equal, "="},
    {Operator::Less, "<"},
    {Operator::More, ">"},
    {Operator::MoreEqual, ">="},
    {Operator::NotEqual, "!="},
    {Operator::LessEqual, "<="},
  };

inline std::string trim(std::string_view sv) {
  auto is_space = [](unsigned char c) { return std::isspace(c); };

  auto begin = std::find_if_not(sv.begin(), sv.end(), is_space);
  auto end   = std::find_if_not(sv.rbegin(), sv.rend(), is_space).base();

  if (begin >= end) return {};
  return std::string(begin, end);
}

} // namespace

template <EntityJson T>
IBaseQuery<T>::IBaseQuery(ISqlExecutor* executor) : executor_(executor) {
  table_name_ = QString::fromStdString(Reflection<T>::meta().table_name);
  involved_tables_.push_back(table_name_);
}

template <EntityJson T>
IBaseQuery<T>& IBaseQuery<T>::from(const std::string& table_name) & {
  std::string table_trimmed = trim(table_name);
  DBC_REQUIRE(!table_trimmed.empty());
  table_name_ = QString::fromStdString(table_trimmed);
  involved_tables_.push_back(table_name_);
  return *this;
}

template <EntityJson T>
IBaseQuery<T>& IBaseQuery<T>::where(const std::string& field, const std::string& value) & {
  return where(field, QString::fromStdString(value));
}

template <EntityJson T>
IBaseQuery<T>& IBaseQuery<T>::where(const std::string& field, const QVariant& value) & {
  std::string field_trimmed = trim(field);
  filters_.push_back(QString("%1 = ?").arg(QString::fromStdString(field)));
  values_.push_back(value);
  return *this;
}

template <EntityJson T>
IBaseQuery<T>& IBaseQuery<T>::where(const std::string& field, Operator op, const std::string& value) & {
  return where(field, op, QString::fromStdString(value));
}

template <EntityJson T>
IBaseQuery<T>& IBaseQuery<T>::where(const std::string& field, Operator op, const QVariant& value) & {
  auto it = operator_to_sql.find(op);
  std::string field_trimmed = trim(field);
  DBC_REQUIRE(it != operator_to_sql.end());
  DBC_REQUIRE(!field_trimmed.empty());
  DBC_REQUIRE(value.isValid());
  filters_.push_back(QString("%1 %2 ?").arg(QString::fromStdString(field_trimmed))
                         .arg(QString::fromStdString(operator_to_sql.at(op))));
  values_.push_back(value);
  return *this;
}

template <EntityJson T>
IBaseQuery<T>& IBaseQuery<T>::limit(int n) & {
  DBC_REQUIRE(n > 0);
  limit_clause_ = QString("LIMIT %1").arg(n);
  return *this;
}

template <EntityJson T>
IBaseQuery<T>& IBaseQuery<T>::join(const std::string& table,
                 const std::string& first,
                 const std::string& second) & {
  std::string table_trimmed = trim(table);
  DBC_REQUIRE(!table_trimmed.empty());
  std::string first_trimmed = trim(first);
  DBC_REQUIRE(!first_trimmed.empty());
  std::string second_trimmed = trim(second);
  DBC_REQUIRE(!second_trimmed.empty());

  join_clause_ += QString(" JOIN %1 ON %2 = %3")
  .arg(QString::fromStdString(table_trimmed))
      .arg(QString::fromStdString(first_trimmed))
      .arg(QString::fromStdString(second_trimmed));

  involved_tables_.push_back(QString::fromStdString(table_trimmed));
  return *this;
}
