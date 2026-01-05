#ifndef IRATELIMITER_H
#define IRATELIMITER_H

#include <string>

class IRateLimiter {
 public:
  virtual ~IRateLimiter()                   = default;
  virtual bool allow(const std::string& ip) = 0;
};

#endif  // IRATELIMITER_H
