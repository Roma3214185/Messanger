#ifndef QUERYRESULT_H
#define QUERYRESULT_H

#include "metaentity/EntityConcept.h"

template <EntityJson T>
struct SelectResult {
  std::vector<T> result;
  std::string error;
};

template <EntityJson T>
struct DeleteResult {
  bool success{false};
  int affected_rows{0};
  std::string error;
};

template <EntityJson T>
using QueryResult = std::variant<SelectResult<T>, DeleteResult<T>>;

template <typename V, typename Variant>
struct is_in_variant;

template <typename V, typename... Ts>
struct is_in_variant<V, std::variant<Ts...>> : std::disjunction<std::is_same<V, Ts>...> {};

template <typename V, typename Variant>
concept VariantResultInQueryResult = is_in_variant<V, Variant>::value;
// using QueryResult = Result<QueryValue, DbError>;

#endif  // QUERYRESULT_H
