#ifndef IIDGENERATOR_H
#define IIDGENERATOR_H

#include <cstdint>

class IIdGenerator {
 public:
  virtual ~IIdGenerator()        = default;
  virtual long long generateId() = 0;
};

#endif  // IIDGENERATOR_H
