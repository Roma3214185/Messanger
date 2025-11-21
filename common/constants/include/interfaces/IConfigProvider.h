#ifndef ICONFIGPROVIDER_H
#define ICONFIGPROVIDER_H

#include "Routes.h"
#include "ports.h"
#include "codes.h"

class IConfigProvider {
  public:
    virtual ~IConfigProvider() = default;

    virtual const Ports& ports() const = 0;
    virtual const Routes& routes() const = 0;
    virtual const StatusCodes& statusCodes() const = 0;
    virtual const IssueMessages& issueMessages() const = 0;
};

#endif // ICONFIGPROVIDER_H
