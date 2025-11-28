#ifndef MOCKRATELIMITER_H
#define MOCKRATELIMITER_H

#include "interfaces/IRateLimiter.h"

class MockRateLimiter : public IRateLimiter {
  public:
    bool should_fail = false;
    std::string last_ip;
    int call_allow = 0;

    bool allow(const std::string& ip) override {
      last_ip = ip;
      ++call_allow;
      return !should_fail;
    }
};

#endif // MOCKRATELIMITER_H
