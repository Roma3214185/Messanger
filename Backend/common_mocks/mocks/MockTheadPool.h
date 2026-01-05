#ifndef MOCKTHEADPOOL_H
#define MOCKTHEADPOOL_H

#include "interfaces/IThreadPool.h"

class MockThreadPool : public IThreadPool {
 public:
  int call_count = 0;

 protected:
  void enqueueTask(std::function<void()> task) override {
    ++call_count;
    task();
  }
};

#endif  // MOCKTHEADPOOL_H
