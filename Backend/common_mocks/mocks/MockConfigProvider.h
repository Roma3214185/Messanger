#ifndef MOCKCONFIGPROVIDER_H
#define MOCKCONFIGPROVIDER_H

#include "interfaces/IConfigProvider.h"

class MockConfigProvider : public IConfigProvider {
public:
  MockConfigProvider() = default;
  explicit MockConfigProvider(Ports ports) : mock_ports(ports) {}
  explicit MockConfigProvider(StatusCodes codes) : mock_codes(codes) {}

  const Routes &routes() const override { return mock_routes; }

  const Ports &ports() const override { return mock_ports; }

  const StatusCodes &statusCodes() const override { return mock_codes; }

  const IssueMessages &issueMessages() const override {
    return mock_issue_message;
  }

  Ports mock_ports;
  StatusCodes mock_codes;
  Routes mock_routes;
  IssueMessages mock_issue_message;
};

#endif // MOCKCONFIGPROVIDER_H
