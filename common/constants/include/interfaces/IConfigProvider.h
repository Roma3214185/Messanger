#ifndef ICONFIGPROVIDER_H
#define ICONFIGPROVIDER_H

#include "Routes.h"
#include "codes.h"
#include "ports.h"

class IConfigProvider {
public:
  virtual ~IConfigProvider() = default;

  [[nodiscard]] virtual const Ports &ports() const = 0;
  [[nodiscard]] virtual const Routes &routes() const = 0;
  [[nodiscard]] virtual const StatusCodes &statusCodes() const = 0;
  [[nodiscard]] virtual const IssueMessages &issueMessages() const = 0;
};

#endif // ICONFIGPROVIDER_H
