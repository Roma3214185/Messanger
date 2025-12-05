#ifndef CODES_H
#define CODES_H

#include <string>

struct StatusCodes {
  int success = 200;
  int serverError = 500;
  int userError = 400;
  int badRequest = 400;
  int unauthorized = 401;
  int conflict = 409;
  int notFound = 404;
  int rateLimit = 429;
};

struct IssueMessages {
  std::string userNotFound = "User not founded";
  std::string invalidToken = "Invalid or expired token";
  std::string rateLimitExceed = "Rate limit exceeded";
};

#endif // CODES_H
