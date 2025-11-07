#ifndef ICACHESERVICE_H
#define ICACHESERVICE_H

#include <string>

class ICacheService {
  public:
    virtual ~ICacheService() = default;
    virtual void remove(const std::string& key) = 0;
    virtual void incr(const std::string& key) = 0;
    virtual bool exists(const std::string& key) = 0;
};

#endif // ICACHESERVICE_H
