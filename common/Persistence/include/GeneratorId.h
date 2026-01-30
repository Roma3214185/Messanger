#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>

#include "interfaces/IIdGenerator.h"

class GeneratorId : public IIdGenerator {
 public:
  explicit GeneratorId(uint8_t serviceId);

  long long generateId() override;

 private:
  static constexpr uint64_t SEQ_BITS = 14;
  static constexpr uint64_t SERVICE_BITS = 8;
  static constexpr uint64_t TIME_SHIFT = SEQ_BITS + SERVICE_BITS;
  static constexpr uint64_t SERVICE_SHIFT = SEQ_BITS;
  static constexpr uint64_t SEQ_MASK = (1ULL << SEQ_BITS) - 1;

  uint64_t lastTs = 0;
  uint64_t seq = 0;
  uint8_t serviceId;
  std::mutex mtx;
};
