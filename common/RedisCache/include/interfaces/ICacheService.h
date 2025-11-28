#ifndef ICACHESERVICE_H
#define ICACHESERVICE_H

#include <string>

class ICacheService {
 public:
  virtual ~ICacheService()                                             = default;
  virtual void                          clearPrefix(const std::string& key) = 0;
  virtual void                          clearCache() = 0;
  virtual void                          remove(const std::string& key) = 0;
  virtual void                          incr(const std::string& key)   = 0;
  virtual bool                          exists(const std::string& key) = 0;
  virtual std::optional<std::string> get(const std::string& key)    = 0;
  virtual void                          set(const std::string&        key,
                                            const std::string&     value,
                                            std::chrono::milliseconds ttl = std::chrono::hours(24)) = 0;
  virtual void                          setPipelines(const std::vector<std::string>&    keys,
                                                     const std::vector<std::string>& results,
                                                     std::chrono::minutes               ttl = std::chrono::minutes(30)) = 0;
};

#endif  // ICACHESERVICE_H
