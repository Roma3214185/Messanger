#ifndef MOCKMETRICS_H
#define MOCKMETRICS_H

#include "interfaces/IMetrics.h"
#include "MetricsTracker.h"

class MockMetrics : public IMetrics {
  public:
    int call_requestEnded = 0;
    int call_userConnected = 0;
    int call_newMessage = 0;
    int call_userDisconnected = 0;
    int call_newRequest = 0;
    int call_saveRequestLatency = 0;

    void requestEnded(const std::string& method, int res_code, bool hitKey) override {
      ++call_requestEnded;
    }

    void userConnected() override {
      ++call_userConnected;
    }

    void newMessage(const std::string& ip) override {
      ++call_newMessage;
    }

    void userDisconnected() override {
      ++call_userDisconnected;
    }

    void newRequest(const std::string& path) override {
      ++call_newRequest;
    }

    void saveRequestLatency(const double latency) override {
      ++call_saveRequestLatency;
    }

    std::unique_ptr<MetricsTracker> getTracker(const std::string& path) override { return std::make_unique<MetricsTracker>(this, path); }
};

#endif // MOCKMETRICS_H
