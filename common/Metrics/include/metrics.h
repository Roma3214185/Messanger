// #ifndef METRICS_H
// #define METRICS_H

// #include <prometheus/counter.h>
// #include <prometheus/exposer.h>
// #include <prometheus/histogram.h>
// #include <prometheus/registry.h>

// #include <memory>

// // #include "metrics.h"
// #include <prometheus/exposer.h>

// #include <memory>
// #include <string>

// using namespace prometheus;

// class MetricsManager {
//  public:
//   std::shared_ptr<Registry> registry;
//   Counter&                  request_counter;
//   Counter&                  error_counter;
//   Histogram&                request_latency;

//   MetricsManager()
//       : registry(std::make_shared<Registry>()),
//         request_counter(BuildCounter()
//                             .Name("http_requests_total")
//                             .Help("Total HTTP requests")
//                             .Register(*registry)
//                             .Add({{"endpoint", "all"}})),
//         error_counter(BuildCounter()
//                           .Name("http_errors_total")
//                           .Help("Total HTTP errors")
//                           .Register(*registry)
//                           .Add({{"endpoint", "all"}})),
//         request_latency(BuildHistogram()
//                             .Name("http_request_latency_seconds")
//                             .Help("Request latency in seconds")
//                             .Register(*registry)
//                             .Add({}, Histogram::BucketBoundaries{0.001, 0.01, 0.1, 0.5, 1, 2, 5})) {
//   }
// };

// class GlobalMetrics {
//  public:
//   static MetricsManager& metrics() {
//     static MetricsManager instance;
//     return instance;
//   }

//   static void initExposer(unsigned short port = 8080) {
//     static bool initialized = false;
//     if (!initialized) {
//       exposer_ = std::make_unique<prometheus::Exposer>("127.0.0.1:" + std::to_string(port));
//       exposer_->RegisterCollectable(metrics().registry);
//       initialized = true;
//     }
//   }

//  private:
//   GlobalMetrics() = default;
//   static inline std::unique_ptr<prometheus::Exposer> exposer_;
// };

// // #include <atomic>
// // #include <unordered_map>
// // #include <mutex>
// // #include <string>
// // //#include "DebugProfiling/Debug_profiling.h"

// // class MetricsImpl;

// // class Metrics {
// //   public:
// //     Metrics();
// //     ~Metrics();

// //     void* getHistogram() const;
// //     void* getCounter() const;

// //   private:
// //     std::unique_ptr<MetricsImpl> impl_;
// // };

// #endif  // METRICS_H
