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
      return ports_;
    }

    const Routes& routes() const override {
      return routes_;
    }

    const StatusCodes& statusCodes() const override {
      return codes_;
    }

  private:
    Ports ports_;
    Routes routes_;
    StatusCodes codes_;
};

#endif // PRODCONFIGPROVIDER_H
