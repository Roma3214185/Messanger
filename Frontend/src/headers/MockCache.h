#ifndef MOCKCACHE_H
#define MOCKCACHE_H

#include <unordered_map>

#include "ICache.h"

using CacheMap = std::unordered_map<Key, Token>;

class MockCache : public ICache {
 public:
  OptionalToken get(const Key& key) override {
    auto iter = cache_.find(key);
    if (iter != cache_.end()) return it->second;
    return std::nullopt;
  }

  void saveToken(const Key& key, const Token& token) override {
    cache_[key] = token;
  }

  void deleteToken(const Key& key) override { cache_.erase(key); }

 private:
  CacheMap cache_;
};

#endif  // MOCKCACHE_H
