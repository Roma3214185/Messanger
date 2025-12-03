#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#ifdef ENABLE_TRACY
#include "../external/tracy/public/tracy/Tracy.hpp"
#define PROFILE_SCOPE(name) ZoneScopedN(name)
#else
#define PROFILE_SCOPE(name) ScopedTimer timer##__LINE__(name)
// #define PROFILE_SCOPE_WITH_METRICS(name) ScopedTimer timer##__LINE__(name, true)
#endif

#define LOG_INFO(...) spdlog::info(__VA_ARGS__)
#define LOG_WARN(...) spdlog::warn(__VA_ARGS__)
#define LOG_ERROR(...) spdlog::error(__VA_ARGS__)
#define LOG_DEBUG(...) spdlog::debug(__VA_ARGS__)

class ScopedTimer {
  std::string                                    name_;
  std::chrono::high_resolution_clock::time_point start_;

 public:
  explicit ScopedTimer(std::string name)
      : name_(std::move(name)), start_(std::chrono::high_resolution_clock::now()) {
    spdlog::info("[TIMER START] {}", name_);
  }

  ~ScopedTimer() noexcept {
    auto   end              = std::chrono::high_resolution_clock::now();
    double duration_seconds = std::chrono::duration<double>(end - start_).count();

    try {
      spdlog::info("[TIMER END] {} took {:.3f} s", name_, duration_seconds);
    } catch (...) {
      LOG_ERROR("Error in destructor");
    }
    // if (record_metrics_) {
    //   auto& metrics = GlobalMetrics::metrics();
    //   metrics.request_counter.Increment();
    //   metrics.request_latency.Observe(duration_seconds);
    // }
  }
};

// inline void enable_sqlite_logging(sqlite3* db) {
//     sqlite3_trace_v2(db, SQLITE_TRACE_STMT, [](unsigned, void*, void* p,
//     void*) -> int {
//         std::cout << "[SQL QUERY] " << static_cast<const char*>(p) <<
//         std::endl; return 0;
//     }, nullptr);
// }

inline void measure_network_call(const std::string& name, const std::function<void()>& func) {
  auto start = std::chrono::high_resolution_clock::now();
  func();
  auto end = std::chrono::high_resolution_clock::now();
  spdlog::info("[NETWORK] {} took {} ms",
               name,
               std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
}

inline void init_logger(const std::string& service_name) {
  try {
    constexpr int fiveMB = 5 * 1024 * 1024;
    constexpr size_t  kMaxLogFileSize = fiveMB;
    constexpr size_t  kMaxLogFiles    = 3;
    const std::string kLogDirectory   = "logs/";
    const std::string kLogExtension   = ".log";
    const std::string kLogPattern     = "[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] %v";

    std::string filename = kLogDirectory + service_name + kLogExtension;

    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        filename, kMaxLogFileSize, kMaxLogFiles));

    auto logger = std::make_shared<spdlog::logger>(service_name, sinks.begin(), sinks.end());
    spdlog::set_default_logger(logger);
    spdlog::set_pattern(kLogPattern);
    spdlog::set_level(spdlog::level::info);

    spdlog::info("Logger initialized for service '{}'", service_name);
  } catch (const spdlog::spdlog_ex& ex) {
    std::cerr << "Log init failed: " << ex.what() << std::endl;
  }
}
