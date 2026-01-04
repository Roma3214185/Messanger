#ifndef BACKEND_GENERICREPOSITORY_IENTITYBUILDER_H_
#define BACKEND_GENERICREPOSITORY_IENTITYBUILDER_H_

#include <QtSql/qsqlquery.h>

#include <QDateTime>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

#include "Meta.h"

template <typename T>
class IEntityBuilder {
 public:
  virtual ~IEntityBuilder()               = default;
  virtual T build(QSqlQuery& query) const = 0;
};

template <typename T>
class MetaEntityBuilder : public IEntityBuilder<T> {
 public:
  T build(QSqlQuery& query) const override {
    const auto& meta = Reflection<T>::meta();
    T           entity;
    for (const auto& f : meta.fields) {
      QVariant v = query.value(f.name);
      if (!v.isValid()) continue;

      auto val = [f, v]() -> std::any {
        if (f.type == typeid(long long)) return v.toLongLong();
        if (f.type == typeid(std::string)) return v.toString().toStdString();
        if (f.type == typeid(QDateTime)) return v.toDateTime();

        LOG_ERROR("In build MetaEntityBuilder invalid type: {}", f.type.name());
        return {};
      }();

      f.set(&entity, val);
    }
    return entity;
  }
};

template <typename T>
class FastEntityBuilder : public IEntityBuilder<T> {
 public:
  T build(QSqlQuery& query) const override { return Builder<T>::build(query); }
};

template <typename T, auto& Fields>
class GenericFastEntityBuilder : public IEntityBuilder<T> {
 public:
  T build(QSqlQuery& query) const override {
    return FastBuilder<T, decltype(Fields)>::build(query, Fields);
  }
};

enum class BuilderType : std::uint8_t { Meta, Fast, Generic };

template <typename T>
using BuilderFactoryFn = std::function<std::unique_ptr<IEntityBuilder<T>>()>;

template <typename T>
static const std::unordered_map<BuilderType, BuilderFactoryFn<T>> builder_map = {
    {BuilderType::Meta, [] { return std::make_unique<MetaEntityBuilder<T>>(); }},
    {BuilderType::Fast, [] { return std::make_unique<FastEntityBuilder<T>>(); }},
    {BuilderType::Generic,
     [] { return std::make_unique<GenericFastEntityBuilder<T, EntityFields<T>::fields>>(); }},
};

template <typename T>
std::unique_ptr<IEntityBuilder<T>> makeBuilder(BuilderType type) {
  return builder_map<T>.at(type)();
}

#endif  // BACKEND_GENERICREPOSITORY_IENTITYBUILDER_H_"
