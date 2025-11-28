#ifndef BACKEND_GENERICREPOSITORY_META_H_
#define BACKEND_GENERICREPOSITORY_META_H_

#include <QtSql/QSqlQuery>
#include <any>
#include <functional>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

#include "Debug_profiling.h"

struct Field {
  const char*                                 name;
  const std::type_info&                       type;
  std::function<std::any(const void*)>        get;
  std::function<void(void*, const std::any&)> set;
};

struct Meta {
  const char*        name;
  const char*        table_name;
  std::vector<Field> fields;

  const Field* find(const std::string& n) const {
    for (const auto& f : fields)
      if (n == f.name) return &f;
    return nullptr;
  }
};

template <class T, class M>
Field make_field(const char* name, M T::* member) {
  return Field{.name = name,
               .type = typeid(M),
               .get  = [member](const void* obj) -> std::any {
                 const T* element = static_cast<const T*>(obj);
                 return element->*member;
               },
               .set =
                   [member](void* obj, const std::any& val) {
                     T* element = static_cast<T*>(obj);

                       // if constexpr (std::is_same_v<M, std::string>) {
                       //    if (val.type() == typeid(const char*)) {
                       //     (element->*member) = std::string(std::any_cast<const char*>(val));
                       //    }
                       //    else
                      (element->*member) = std::any_cast<M>(val);
                       //} /
                     //}
                   }};
}

template <typename T, typename FieldTuple>
struct FastBuilder {
  static T build(QSqlQuery& query, const FieldTuple& fields) {
    T   entity;
    int i = 0;

    auto assign = [&](auto ptr) {
      using MemberType = std::decay_t<decltype(entity.*ptr)>;
      QVariant value   = query.value(i++);
      if constexpr (std::is_same_v<MemberType, long long>)
        entity.*ptr = value.toLongLong();
      else if constexpr (std::is_same_v<MemberType, int>)
        entity.*ptr = value.toInt();
      else if constexpr (std::is_same_v<MemberType, std::string>)
        entity.*ptr = value.toString().toStdString();
      else if constexpr (std::is_same_v<MemberType, QString>)
        entity.*ptr = value.toString();
      else
        entity.*ptr = value.value<MemberType>();
    };

    std::apply([&](auto... ptrs) { (assign(ptrs), ...); }, fields);

    return entity;
  }
};

template <typename T>
struct Reflection {
  static Meta meta();
};

template <typename T>
struct Builder {
  T build(QSqlQuery& query);
};

template <typename T>
struct EntityFields;

template <typename T>
struct EntityKey {
  static std::string get(const T& entity) { return std::to_string(entity.id); }
};

#endif  // BACKEND_GENERICREPOSITORY_META_H_
