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

    const IssueMessages& issueMessages() const override {
      return messages_;
    }

  private:
    Ports ports_;
    Routes routes_;
    StatusCodes codes_;
    IssueMessages messages_;
};

#endif // PRODCONFIGPROVIDER_H
