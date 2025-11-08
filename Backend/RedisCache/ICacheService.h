#ifndef ICACHESERVICE_H
#define ICACHESERVICE_H

#include <string>
#include <nlohmann/json.hpp>

class ICacheService {
  public:
    virtual ~ICacheService() = default;
    virtual void remove(const std::string& key) = 0;
    virtual void incr(const std::string& key) = 0;
    virtual bool exists(const std::string& key) = 0;
    virtual std::optional<nlohmann::json> get(const std::string& key) = 0;
    virtual void set(const std::string& key, const nlohmann::json& value,
                     std::chrono::milliseconds ttl = std::chrono::hours(24)) = 0;
};

#endif // ICACHESERVICE_H
