#ifndef TRACY_H
#define TRACY_H

#include <fmt/base.h>
#include <fmt/format.h>
#include <opentelemetry/exporters/otlp/otlp_grpc_exporter.h>
#include <opentelemetry/exporters/otlp/otlp_grpc_exporter_options.h>
#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/sdk/trace/exporter.h>
#include <opentelemetry/sdk/trace/processor.h>
#include <opentelemetry/sdk/trace/simple_processor.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/trace/provider.h>
#include <opentelemetry/trace/scope.h>
#include <opentelemetry/trace/span.h>
#include <opentelemetry/trace/tracer.h>
#include <opentelemetry/trace/tracer_provider.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <memory>
#include <string>
#include <utility>

namespace trace    = opentelemetry::trace;
namespace sdktrace = opentelemetry::sdk::trace;
namespace nostd    = opentelemetry::nostd;

inline nostd::shared_ptr<trace::Tracer> initTracer(const int port = 4317) {
  opentelemetry::exporter::otlp::OtlpGrpcExporterOptions options;
  options.endpoint = "localhost:" + std::to_string(port);

  static auto tracer = [&options]() {
    auto exporter = std::unique_ptr<sdktrace::SpanExporter>(
        std::make_unique<opentelemetry::exporter::otlp::OtlpGrpcExporter>(options));
    auto processor = std::unique_ptr<sdktrace::SpanProcessor>(
         std::make_unique<sdktrace::SimpleSpanProcessor>(std::move(exporter)));
    auto provider = std::shared_ptr<trace::TracerProvider>(
         std::make_unique<sdktrace::TracerProvider>(std::move(processor)));
    trace::Provider::SetTracerProvider(provider);

    return provider->GetTracer("MyTracer");
  }();

  return tracer;
}

class ScopedTimer final {
  std::string                                    name_;
  std::chrono::high_resolution_clock::time_point start_;
  nostd::shared_ptr<trace::Tracer>               tracer_;
  // nostd::shared_ptr<trace::Span> span_;
  // trace::Scope scope_;

 public:
  explicit ScopedTimer(std::string name)
      : name_(std::move(name)),
        start_(std::chrono::high_resolution_clock::now())
  //, tracer_(initTracer())
  //, span_(tracer_->StartSpan(name_))
  //, scope_(tracer_->WithActiveSpan(span_))
  {}

  ScopedTimer(const ScopedTimer&)            = delete;
  ScopedTimer& operator=(const ScopedTimer&) = delete;

  ScopedTimer(ScopedTimer&&)            = delete;
  ScopedTimer& operator=(ScopedTimer&&) = delete;

  ~ScopedTimer() noexcept {
    const auto   end              = std::chrono::high_resolution_clock::now();
    const double duration_seconds = std::chrono::duration<double>(end - start_).count();

    // span_->End();
    spdlog::info("{} took {:.3f} s", name_, duration_seconds);
  }
};

#endif  // TRACY_H
