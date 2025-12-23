#ifndef MOCKAPICACHE_H
#define MOCKAPICACHE_H

#include "interfaces/ICacheService.h"

class MockApiCache : public ICacheService {
  public:
    std::string last_key_to_get;
    std::optional<std::string> mock_answer;

    std::optional<std::string> get(const std::string& key)    override {
      last_key_to_get = key;
      return mock_answer;
    }

    int call_set = 0;
    std::string last_set_key;
    std::string last_set_value;

    void set(const std::string&        key,
             const std::string&     value,
             std::chrono::seconds ttl = std::chrono::hours(24)) override {
      ++call_set;
      last_set_key = key;
      last_set_value = value;
    }

  private:
    void clearPrefix(const std::string& key) override {

    }

    void clearCache() override {

    }

    void remove(const std::string& key) override {

    }

    void incr(const std::string& key)   override {

    }

    bool exists(const std::string& key) override {

    }

    void setPipelines(const std::vector<std::string>&    keys,
                      const std::vector<std::string>& results,
                      std::chrono::seconds               ttl = std::chrono::minutes(30)) override {

    }
};

#endif // MOCKAPICACHE_H
