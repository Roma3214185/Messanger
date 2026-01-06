#ifndef MOCKIDGENERATOR_H
#define MOCKIDGENERATOR_H

#include "interfaces/IIdGenerator.h"

struct MockIdGenerator : public IIdGenerator {
  long long mocked_id = -1;
  long long generateId() override { return mocked_id; }
};

#endif // MOCKIDGENERATOR_H
