#ifndef METRICSTRACKER_H
#define METRICSTRACKER_H

#include <chrono>

#include "interfaces/IMetrics.h"

struct MetricsTracker {
  IMetrics* metrics_ = nullptr;
  std::chrono::steady_clock::time_point start;
  MetricsTracker() = default;

  void startTimer(IMetrics* metrics);

  MetricsTracker(const MetricsTracker&) = delete;
  MetricsTracker& operator=(const MetricsTracker&) = delete;

  MetricsTracker(MetricsTracker&& other) noexcept;
  MetricsTracker& operator=(MetricsTracker&& other) noexcept;

  ~MetricsTracker();
};

#endif  // METRICSTRACKER_H
