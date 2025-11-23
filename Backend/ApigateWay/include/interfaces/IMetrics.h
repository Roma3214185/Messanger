#ifndef APIGATE_IMETRICS_H
#define APIGATE_IMETRICS_H

#include <string>
#include <memory>

struct MetricsTracker;

class IMetrics {
  public:
    virtual ~IMetrics() = default;
    virtual void requestEnded(const std::string& method, int res_code, bool hitKey) = 0;
    virtual void userConnected() = 0;
    virtual void newMessage(const std::string& ip) = 0;
    virtual void userDisconnected() = 0;
    virtual void newRequest(const std::string& path) = 0;
    virtual void saveRequestLatency(const double latency) = 0;
    virtual std::unique_ptr<MetricsTracker> getTracker(const std::string& path) = 0;
};

#endif // APIGATE_IMETRICS_H
