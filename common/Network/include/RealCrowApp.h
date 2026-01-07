#ifndef CROWAPPWRAPPER_H
#define CROWAPPWRAPPER_H

#include <crow.h>

#include "interfaces/IApp.h"

class CrowAppWrapper : public IApp {
public:
  explicit CrowAppWrapper(crow::SimpleApp &app) : app_(app) {}

  IApp &port(int p) override {
    app_.port(p);
    return *this;
  }

  IApp &multithreaded() override {
    app_.multithreaded();
    return *this;
  }

  void run() override { app_.run(); }

private:
  crow::SimpleApp &app_;
};

#endif // CROWAPPWRAPPER_H
