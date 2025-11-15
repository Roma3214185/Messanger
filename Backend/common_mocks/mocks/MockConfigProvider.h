#ifndef MOCKCONFIGPROVIDER_H
#define MOCKCONFIGPROVIDER_H

#include "interfaces/IConfigProvider.h"

class MockConfigProvider : public IConfigProvider {
  public:
    const Routes& routes() const override {
      return mock_routes;
    }

    const Ports& ports() const override {
      return mock_ports;
    }

    const StatusCodes& statusCodes() const override {
      return mock_codes;
    }

    Ports mock_ports;
    StatusCodes mock_codes;
    Routes mock_routes;
};

#endif // MOCKCONFIGPROVIDER_H
