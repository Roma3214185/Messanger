#ifndef UTILS_H
#define UTILS_H

#include <optional>
#include <nlohmann/json.hpp>
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

inline std::optional<long long> getIdFromStr(const std::string &str) {
  try {
    return std::stoll(str);
  } catch (...) {
    LOG_ERROR("Error while get stoll of {}", str);
    return std::nullopt;
  }
}


} // namespace utils


#endif // UTILS_H
