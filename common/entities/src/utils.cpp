#include "utils.h"

namespace utils {

std::optional<long long> getIdFromStr(const std::string &str) {
  try {
    return std::stoll(str);
  } catch (...) {
    LOG_ERROR("Error while get stoll of {}", str);
    return std::nullopt;
  }
}

long long getCurrentTime() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
      .count();
}

void addFiledToJson(nlohmann::json& josn, const std::string &field, const std::string &value) {
    josn[field] = value;
}

}  // namespace utils
