#ifndef SIGNUPREQUEST_H
#define SIGNUPREQUEST_H

#include <QString>

struct SignUpRequest {
  QString email{};
  QString password{};
  QString tag{};
  QString name{};
};

struct LogInRequest {
  QString email{};
  QString password{};
};

#endif  // SIGNUPREQUEST_H
