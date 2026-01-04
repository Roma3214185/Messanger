#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>

#include "interfaces/IIdGenerator.h"

class GeneratorId : public IIdGenerator {
 public:
  explicit GeneratorId(uint8_t serviceId) : serviceId(serviceId) {}

  long long generateId() override {
    using namespace std::chrono;
    std::lock_guard<std::mutex> lock(mtx);

    uint64_t now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    if (now == lastTs) {
      seq = (seq + 1) & SEQ_MASK;
      if (seq == 0) {
        while (now <= lastTs) {
          now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        }
      }
    } else {
      lastTs = now;
      seq    = 0;
    }

    return (long long)(now << TIME_SHIFT) | (static_cast<uint64_t>(serviceId) << SERVICE_SHIFT) |
           seq;
  }

 private:
  static constexpr uint64_t SEQ_BITS      = 14;
  static constexpr uint64_t SERVICE_BITS  = 8;
  static constexpr uint64_t TIME_SHIFT    = SEQ_BITS + SERVICE_BITS;
  static constexpr uint64_t SERVICE_SHIFT = SEQ_BITS;
  static constexpr uint64_t SEQ_MASK      = (1ULL << SEQ_BITS) - 1;

  uint64_t   lastTs = 0;
  uint64_t   seq    = 0;
  uint8_t    serviceId;
  std::mutex mtx;
};
