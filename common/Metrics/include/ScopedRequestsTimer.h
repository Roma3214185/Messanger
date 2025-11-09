#ifndef SCOPEDREQUESTSTIMER_H
#define SCOPEDREQUESTSTIMER_H

#include <prometheus/counter.h>
#include <prometheus/exposer.h>
#include <prometheus/histogram.h>
#include <prometheus/registry.h>

#include <chrono>
#include <string>

class ScopedRequestMetrics {
 public:
  ScopedRequestMetrics(prometheus::Family<prometheus::Counter>& counter_family,
                       prometheus::Histogram&                   histogram,
                       const std::string&                       route,
                       const std::string&                       method)
      : counter_family_(counter_family),
        histogram_(histogram),
        route_(route),
        method_(method),
        start_(std::chrono::steady_clock::now()),
        status_code_(0) {}

  void setStatus(int code) { status_code_ = code; }

  ~ScopedRequestMetrics() {
    auto& counter = counter_family_.Add(
        {{"route", route_}, {"method", method_}, {"status", std::to_string(status_code_)}});
    counter.Increment();

    auto                          end     = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = end - start_;
    histogram_.Observe(elapsed.count());
  }

 private:
  prometheus::Family<prometheus::Counter>&           counter_family_;
  prometheus::Histogram&                             histogram_;
  std::string                                        route_;
  std::string                                        method_;
  int                                                status_code_;
  std::chrono::time_point<std::chrono::steady_clock> start_;
};

#endif  // SCOPEDREQUESTSTIMER_H
