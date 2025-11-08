#ifndef MOCKCACHE_H
#define MOCKCACHE_H

#include "ICacheService.h"
#include <vector>
#include <string>

class MockCache : public ICacheService {
  public:
    std::vector<std::string> incr_calls;
    std::vector<std::string> remove_calls;

    void incr(const std::string& key) override {
      incr_calls.push_back(key);
    }

    void remove(const std::string& key) override {
      remove_calls.push_back(key);
    }

    bool wasIncrCalledWith(const std::string& key) const {
      return std::find(incr_calls.begin(), incr_calls.end(), key) != incr_calls.end();
    }

    bool wasRemoveCalledWith(const std::string& key) const {
      return std::find(remove_calls.begin(), remove_calls.end(), key) != remove_calls.end();
    }
};

#endif // MOCKCACHE_H
