#ifndef METRICSTRACKER_H
#define METRICSTRACKER_H

#include <chrono>

#include "interfaces/IMetrics.h"

struct MetricsTracker {
    IMetrics* metrics_;
    std::chrono::steady_clock::time_point start;
    std::string path_;

    MetricsTracker(IMetrics* metrics, const std::string& path)
        : metrics_(metrics)
        , path_(path)
        , start(std::chrono::steady_clock::now()) {

      metrics_->newRequest(path);
    }

    ~MetricsTracker() {
      using namespace std::chrono;

      auto end = steady_clock::now();
      auto latency = duration<double>(end - start).count();
      metrics_->saveRequestLatency(latency);
    }
};

#endif // METRICSTRACKER_H
