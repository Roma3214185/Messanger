#ifndef REDISCLIENT_H
#define REDISCLIENT_H

#include <sw/redis++/redis++.h>

#include "Debug_profiling.h"
#include "interfaces/ICache.h"

class RedisClient : public ICache {
 public:
  explicit RedisClient(const std::string& url);

  OptionalToken get(const Key &key) override;

  void saveToken(const Key &key, const TokenStd &token) override;

  void deleteToken(const Key &key) override;

 private:
  sw::redis::Redis redis;  // todo: Impl idiom to remove #include <sw/redis++/redis++.h>
};

#endif  // REDISCLIENT_H
