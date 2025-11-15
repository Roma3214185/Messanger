#ifndef MOCKCONFIGPROVIDER_H
#define MOCKCONFIGPROVIDER_H

#include "interfaces/IConfigProvider.h"

class MockConfigProvider : public IConfigProvider {
  public:
    const Routes& routes() const override {
      return mock_routes;
    }

    const Ports& ports() const override {
      Ports ports;
      return ports;
    }

    const StatusCodes& statusCodes() const override {
      StatusCodes codes;
      return codes;
    }

    Routes mock_routes;
};

#endif // MOCKCONFIGPROVIDER_H
