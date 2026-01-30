#include "GeneratorId.h"

namespace {

uint64_t get_now_time() {
  return static_cast<uint64_t>(
      std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
          .count());
}

}  // namespace

GeneratorId::GeneratorId(uint8_t serviceId) : serviceId(serviceId) {}

long long GeneratorId::generateId() {
  std::scoped_lock lock(mtx);

  uint64_t now = get_now_time();

  if (now == lastTs) {
    seq = (seq + 1) & SEQ_MASK;
    if (seq == 0) {
      while (now <= lastTs) {
        now = get_now_time();
      }
    }
  } else {
    lastTs = now;
    seq = 0;
  }

  return static_cast<long long>((now << TIME_SHIFT) | (static_cast<uint64_t>(serviceId) << SERVICE_SHIFT) | seq);
}
