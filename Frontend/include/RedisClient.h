#ifndef REDISCLIENT_H
#define REDISCLIENT_H

#include "interfaces/ICache.h"

class RedisClient : public ICache {
 public:
  explicit RedisClient(const std::string &url);
  ~RedisClient();

  OptionalToken get(const Key &key) override;
  void saveToken(const Key &key, const TokenStd &token) override;
  void deleteToken(const Key &key) override;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

#endif  // REDISCLIENT_H
