#ifndef LOGGER_H
#define LOGGER_H

#include <crow.h>

#include "RequestDTO.h"

class Logger {
  public:
    void logRequest(const crow::request& req, const RequestDTO&);
    void logResponse(int status, const std::string& body, const RequestDTO&, bool cacheHit);
};

#endif // LOGGER_H
