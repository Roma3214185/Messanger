#ifndef MOCKAPPWRAPPER_H
#define MOCKAPPWRAPPER_H

#include <crow.h>
#include "interfaces/IApp.h"

class MockAppWrapper : public IApp {
    crow::SimpleApp& app_;
  public:
    int last_port;
    int call_run = 0;
    int call_multithread = 0;

    MockAppWrapper(crow::SimpleApp& app) : app_(app) { }

    virtual MockAppWrapper& port(int p) override{
      last_port = p;
      return *this;
    }

    virtual MockAppWrapper& multithreaded() override{
      ++call_multithread;
      return this*;
    }

    void run() override {
      ++call_run;
    }
};

#endif // MOCKAPPWRAPPER_H
