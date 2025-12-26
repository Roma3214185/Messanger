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
#include <filesystem>
#include <sstream>
#include "tracy.h"

//todo: remove ScopedTimer from here, make file Logger and file Tracer, and extract_class_and_function in separate namespace

class ScopedTimer;

#define PROFILE_SCOPE(name) ScopedTimer timer##__LINE__(extract_class_and_function(__func__, __PRETTY_FUNCTION__).c_str())

inline std::string extract_class_and_function(const char* func_name, const char* full_func) {
  std::string full(full_func);
  std::string name(func_name);
  std::stringstream ss(full);
  std::string token;

  while (ss >> token) {
    if (token.find(name) != std::string::npos) {
      std::string ans = token.substr(0, token.find('('));
      return ans;
    }
  }
  return name;
}

#ifdef NDEBUG
#define LOG_INFO(...) ((void)0)
//#define LOG_INFO(...) \
//spdlog::log(spdlog::source_loc{__FILE__, __LINE__, extract_class_and_function(__func__, __PRETTY_FUNCTION__).c_str()}, spdlog::level::info, __VA_ARGS__)
#else
#define LOG_INFO(...) \
spdlog::log(spdlog::source_loc{__FILE__, __LINE__, extract_class_and_function(__func__, __PRETTY_FUNCTION__).c_str()}, spdlog::level::info, __VA_ARGS__)
#endif

#define LOG_WARN(...) \
spdlog::log(spdlog::source_loc{__FILE__, __LINE__, extract_class_and_function(__func__, __PRETTY_FUNCTION__).c_str()}, spdlog::level::warn, __VA_ARGS__)

#define LOG_ERROR(...) \
spdlog::log(spdlog::source_loc{__FILE__, __LINE__, extract_class_and_function(__func__, __PRETTY_FUNCTION__).c_str()}, spdlog::level::err, __VA_ARGS__)

#ifdef NDEBUG
 #define LOG_DEBUG(...) ((void)0)
#else
 #define LOG_DEBUG(...) \
 spdlog::log(spdlog::source_loc{__FILE__, __LINE__, extract_class_and_function(__func__, __PRETTY_FUNCTION__).c_str()}, spdlog::level::debug, __VA_ARGS__)
#endif


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
    constexpr size_t kMaxLogFileSize = fiveMB;
    constexpr size_t kMaxLogFiles = 3;
    const std::string kLogDirectory = "/Users/roma/QtProjects/Chat/logs/";
    const std::string kLogExtension = ".log";
    const std::string kLogPattern = "[%d-%m %H:%M:%S] [%l] [%!] %v";

    std::filesystem::create_directories(kLogDirectory);

    std::string filename = kLogDirectory + service_name + kLogExtension;

    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    sinks.push_back(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
        filename, kMaxLogFileSize, kMaxLogFiles));

    auto logger = std::make_shared<spdlog::logger>(service_name, sinks.begin(), sinks.end());
    spdlog::set_default_logger(logger);
    spdlog::set_pattern(kLogPattern);
    spdlog::set_level(spdlog::level::info);
    spdlog::flush_on(spdlog::level::info);

    spdlog::info("Logger initialized for service '{}'", service_name);
  } catch (const spdlog::spdlog_ex& ex) {
    std::cerr << "Log init failed: " << ex.what() << std::endl;
  }
}
