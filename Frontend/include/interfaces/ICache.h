#ifndef ICACHE_H
#define ICACHE_H

#include <optional>
#include <string>

using Key = std::string;
using Token = std::string;
using OptionalToken = std::optional<Token>;

class ICache {
public:
  ICache() = default;
  ICache(const ICache &) = delete;
  ICache &operator=(const ICache &) = delete;
  ICache(ICache &&) = default;
  ICache &operator=(ICache &&) = default;

  virtual ~ICache() = 0;

  [[nodiscard]] virtual auto get(const Key &key) -> OptionalToken = 0;
  virtual void saveToken(const Key &key, const Token &token) = 0;
  virtual void deleteToken(const Key &key) = 0;
};

inline ICache::~ICache() = default;

#endif // ICACHE_H
