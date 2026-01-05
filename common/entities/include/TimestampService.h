#ifndef TIMESTAMPSERVICE_H
#define TIMESTAMPSERVICE_H

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

namespace TimestampService {

inline static std::time_t parseTimestampISO8601(const std::string& iso_str) {
  std::tm            tm{};
  std::istringstream ss(iso_str);
  ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
  if (ss.fail()) {
    return std::time(nullptr);
  }
  return std::mktime(&tm);
}

inline std::string timestampToISO8601(std::time_t timestamp) {
  std::tm tm{};
#ifdef _WIN32
  gmtime_s(&tm, &timestamp);
#else
  gmtime_r(&timestamp, &tm);
#endif
  std::ostringstream ss;
  ss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
  return ss.str();
}

}  // namespace TimestampService

#endif  // TIMESTAMPSERVICE_H
