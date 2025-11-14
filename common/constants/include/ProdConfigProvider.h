#ifndef PRODCONFIGPROVIDER_H
#define PRODCONFIGPROVIDER_H

#include "interfaces/IConfigProvider.h"

class ProdConfigProvider : public IConfigProvider {
  public:
    static ProdConfigProvider& instance() {
      static ProdConfigProvider inst;
      return inst;
    }

    const Ports& ports() const override {
      return Ports{};
    }

    const Routes& routes() const override {
      return Routes{};
    }

    const StatusCodes& statusCodes() const override {
      return StatusCodes{};
    }
};

#endif // PRODCONFIGPROVIDER_H
