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

#define PROFILE_SCOPE(name) ScopedTimer timer##__LINE__(extract_class_and_function(__PRETTY_FUNCTION__).c_str())

inline std::string extract_class_and_function(const char* full_func) {
  const std::string full(full_func);
  std::stringstream sstream(full);
  std::string token;

  while (sstream >> token) {
    if (token.find('(') != std::string::npos) {
      return token.substr(0, token.find('('));
    }
  }
  return full;
}

// #ifdef __cpp_concepts
// // Concept: has toStdString() method
// template <typename T>
// concept HasToStdString = requires(T a) {
//   { a.toStdString() } -> std::convertible_to<std::string>;
// };

// // Generic to_loggable
// template <typename T>
// auto to_loggable(T&& value) {
//   if constexpr (HasToStdString<T>) {
//     return value.toStdString(); // only called if T has toStdString
//   } else {
//     return std::forward<T>(value); // default
//   }
// }

// #else
// // Fallback: use SFINAE for older compilers
// template <typename T>
// auto to_loggable(T&& value) -> decltype(value.toStdString()) {
//   return value.toStdString();
// }

// template <typename T>
// auto to_loggable(T&& value) -> T&& {
//   return std::forward<T>(value);
// }
// #endif

#ifdef NDEBUG
#define LOG_INFO(...) ((void)0)
//#define LOG_INFO(...) \
//spdlog::log(spdlog::source_loc{__FILE__, __LINE__, extract_class_and_function(__PRETTY_FUNCTION__).c_str()}, spdlog::level::info, __VA_ARGS__)
#else
#define LOG_INFO(...) \
spdlog::log(spdlog::source_loc{__FILE__, __LINE__, extract_class_and_function(__PRETTY_FUNCTION__).c_str()}, spdlog::level::info, __VA_ARGS__)
#endif

#define LOG_WARN(...) \
spdlog::log(spdlog::source_loc{__FILE__, __LINE__, extract_class_and_function(__PRETTY_FUNCTION__).c_str()}, spdlog::level::warn, __VA_ARGS__)

#define LOG_ERROR(...) \
spdlog::log(spdlog::source_loc{__FILE__, __LINE__, extract_class_and_function(__PRETTY_FUNCTION__).c_str()}, spdlog::level::err, __VA_ARGS__)

#ifdef NDEBUG
 #define LOG_DEBUG(...) ((void)0)
constexpr bool kLogEnabled = false;
#else
 #define LOG_DEBUG(...) \
 spdlog::log(spdlog::source_loc{__FILE__, __LINE__, extract_class_and_function(__PRETTY_FUNCTION__).c_str()}, spdlog::level::debug, __VA_ARGS__)
constexpr bool kLogEnabled = true;
#endif



// template <typename T>
// inline auto to_loggable(T&& value) {
//   return std::forward<T>(value);
// }

// template <typename... Args>
// auto makeLogArgs(Args&&... args) {
//   return std::tuple{to_loggable(std::forward<Args>(args))...};
// }

// template <typename... Args>
// inline void logInfo(const char* file, int line, const char* func, Args&&... args) {
//   if constexpr (log_enabled) {
//     spdlog::log(
//         spdlog::source_loc{file, line, extract_class_and_function(func, func).c_str()},
//         spdlog::level::info,
//         fmt::format("{}", to_loggable(std::forward<Args>(args)))...
//         );
//   }
// }

// #ifdef NDEBUG
// #define LOG_INFO(...) logInfo(__FILE__, __LINE__, __func__, __VA_ARGS__)
// #else
// #define LOG_INFO(...) logInfo(__FILE__, __LINE__, __func__, __VA_ARGS__)
// #endif

inline void measureNetworkCall(const std::string& name, const std::function<void()>& func) {
  auto start = std::chrono::high_resolution_clock::now();
  func();
  auto end = std::chrono::high_resolution_clock::now();
  spdlog::info("[NETWORK] {} took {} ms",
               name,
               std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());
}

inline void initLogger(const std::string& service_name) {
  try {
    constexpr int kFiveMB = 5 * 1024 * 1024;
    constexpr size_t kMaxLogFileSize = kFiveMB;
    constexpr size_t kMaxLogFiles = 3;
    constexpr std::string_view kLogDirectory = "/Users/roma/QtProjects/Chat/logs/";
    constexpr std::string_view kLogExtension = ".log";
    const std::string kLogPattern = "[%d-%m %H:%M:%S] [%l] [%!] %v";

    std::filesystem::create_directories(kLogDirectory);

    std::string filename;
    filename.reserve(
        kLogDirectory.size() +
        service_name.size() +
        kLogExtension.size()
        );

    filename.append(kLogDirectory);
    filename.append(service_name);
    filename.append(kLogExtension);

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
    std::cerr << "Log init failed: " << ex.what() << '\n';
  }
}

enum class ContractLevel {
  Precondition,
  Postcondition,
  Invariant,
  Assert,
  Unreachable
};

struct ContractViolation {
    ContractLevel level;
    const char* expr;
    const char* file;
    int line;
    const char* func;
};

using ContractHandler = void(*)(const ContractViolation&);

static const char* to_string(ContractLevel level) {
  switch (level) {
  case ContractLevel::Precondition: return "Precondition";
  case ContractLevel::Postcondition: return "Postcondition";
  case ContractLevel::Invariant: return "Invariant";
  case ContractLevel::Assert: return "Assert";
  case ContractLevel::Unreachable: return "Unreachable";
  }
  return "Unknown";
}

class ContractViolationError : public std::logic_error {
  public:
    ContractViolation v;

    explicit ContractViolationError(const ContractViolation& violation)
        : std::logic_error(build_message(violation)),
        v(violation)
    {}

  private:
    static std::string build_message(const ContractViolation& v) {
      std::ostringstream oss;
      oss << "Contract violation: "
          << to_string(v.level)
          << " failed: " << v.expr
          << " at " << v.file << ":" << v.line
          << " (" << v.func << ")";
      return oss.str();
    }
};

inline void throw_on_violation(const ContractViolation& v) {
  throw ContractViolationError(v);
}

inline void log_contract(const ContractViolation& v) {
  spdlog::error(
      "[CONTRACT] {} failed: {} at {}:{} ({})",
      to_string(v.level),
      v.expr,
      v.file,
      v.line,
      v.func
      );
}

inline ContractHandler& contract_handler() {
  static ContractHandler handler = log_contract;
  return handler;
}

#define CONTRACT_CHECK(level, expr) \
  do {                              \
      if (!(expr)) {                \
        contract_handler()({        \
        level,                      \
        #expr,                      \
        __FILE__,                   \
        __LINE__,                   \
        __func__                    \
        });                         \
  }                                 \
} while (0)

#ifndef ENABLE_CONTRACTS
#define ENABLE_CONTRACTS
#endif

#ifdef ENABLE_CONTRACTS
#define DBC_REQUIRE(expr)      CONTRACT_CHECK(ContractLevel::Precondition, expr)
#define DBC_ENSURE(expr)     CONTRACT_CHECK(ContractLevel::Postcondition, expr)
#define DBC_INVARIANT(expr)    CONTRACT_CHECK(ContractLevel::Invariant, expr)
#define DBC_ASSERT_INTERNAL(expr) \
  CONTRACT_CHECK(ContractLevel::Assert, expr)
#define DBC_UNREACHABLE()      CONTRACT_CHECK(ContractLevel::Unreachable, false)
#else
#define DBC_REQUIRE(expr)      ((void)0)
#define DBC_ENSURE(expr)     ((void)0)
#define DBC_INVARIANT(expr)    ((void)0)
#define DBC_ASSERT_INTERNAL(expr) ((void)0)
#define DBC_UNREACHABLE()      ((void)0)
#endif
