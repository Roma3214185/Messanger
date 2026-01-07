#ifndef MOCKCACHE_H
#define MOCKCACHE_H

#include <unordered_map>

#include "interfaces/ICache.h"

using CacheMap = std::unordered_map<Key, Token>;

class MockCache : public ICache {
public:
  OptionalToken get(const Key &key) override {
    ++get_calls;
    auto iter = cache_.find(key);
    if (iter != cache_.end())
      return iter->second;
    return std::nullopt;
  }

  void saveToken(const Key &key, const Token &token) override {
    ++save_calls;
    cache_[key] = token;
  }

  void deleteToken(const Key &key) override {
    ++delete_calls;
    cache_.erase(key);
  }

  int get_calls = 0;
  int save_calls = 0;
  int delete_calls = 0;

private:
  CacheMap cache_;
};

#endif // MOCKCACHE_H
