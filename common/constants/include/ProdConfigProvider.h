#ifndef PRODCONFIGPROVIDER_H
#define PRODCONFIGPROVIDER_H

#include "interfaces/IConfigProvider.h"

class ProdConfigProvider : public IConfigProvider {
public:
  static ProdConfigProvider &instance() {
    static ProdConfigProvider inst;
    return inst;
  }

  [[nodiscard]] const Ports &ports() const override { return ports_; }

  [[nodiscard]] const Routes &routes() const override { return routes_; }

  [[nodiscard]] const StatusCodes &statusCodes() const override {
    return codes_;
  }

  [[nodiscard]] const IssueMessages &issueMessages() const override {
    return messages_;
  }

private:
  Ports ports_;
  Routes routes_;
  StatusCodes codes_;
  IssueMessages messages_;
};

#endif // PRODCONFIGPROVIDER_H
