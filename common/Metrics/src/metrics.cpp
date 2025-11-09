#include "metrics.h"

class MetricsImpl {
 public:
  std::shared_ptr<prometheus::Registry> registry;
  prometheus::Histogram*                histogram;
  prometheus::Counter*                  counter;

  MetricsImpl() : registry(std::make_shared<prometheus::Registry>()) {
    using namespace prometheus;

    Histogram::BucketBoundaries bucket_bounds{0.001, 0.01, 0.1, 0.5, 1, 2, 5};
    auto&                       histogram_family = BuildHistogram()
                                 .Name("operation_duration_seconds")
                                 .Help("Duration of operations in seconds")
                                 .Register(*registry);
    histogram = &histogram_family.Add({}, bucket_bounds);

    auto& counter_family = BuildCounter()
                               .Name("operation_count_total")
                               .Help("Number of operations")
                               .Register(*registry);
    counter = &counter_family.Add({});
  }
};

// Metrics::Metrics() : impl_(std::make_unique<MetricsImpl>()) {}
// Metrics::~Metrics() = default;

// void* Metrics::getHistogram() const { return impl_->histogram; }
// void* Metrics::getCounter() const { return impl_->counter; }

// Metrics::Metrics() {}
