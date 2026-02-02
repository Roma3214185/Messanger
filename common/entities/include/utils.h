#ifndef COMMON_ENTITIES_INCLUDE_UTILS_H
#define COMMON_ENTITIES_INCLUDE_UTILS_H

#include <nlohmann/json.hpp>
#include <optional>
#include "Debug_profiling.h"

namespace utils {

template <typename T>
inline std::optional<T> parsePayload(const std::string &payload) {
  try {
    return nlohmann::json::parse(payload).get<T>();
  } catch (...) {
    LOG_ERROR("Failed to parse message payload {}", payload);
    return std::nullopt;
  }
}

std::optional<long long> getIdFromStr(const std::string &str);

long long getCurrentTime();


}  // namespace utils

#endif  // COMMON_ENTITIES_INCLUDE_UTILS_H
