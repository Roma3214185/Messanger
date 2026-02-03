#ifndef CACHEKEYGENERATOR_H
#define CACHEKEYGENERATOR_H

#include <string>
#include "Meta.h"
#include "metaentity/EntityConcept.h"

class CacheKeyGenerator {
 public:
  template <EntityJson T>
  static std::string makeKey(const T& entity) {
    EntityKey<T> key_builder;
    return makeKeyImplementation(key_builder.get(entity), std::string(Reflection<T>::meta().table_name));
  }

  template <EntityJson T>
  static std::string makeKey(long long id) {
    return makeKeyImplementation(std::to_string(id), std::string(Reflection<T>::meta().table_name));
  }

 private:
  static std::string makeKeyImplementation(const std::string& entity_key, const std::string& table_name) {
    return "entity_cache:" + table_name + ":" + entity_key;
  }
};

#endif  // CACHEKEYGENERATOR_H
