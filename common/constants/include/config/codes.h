#ifndef CODES_H
#define CODES_H

#include <string>

namespace Config::StatusCodes {
static constexpr int success = 200;
static constexpr int accepted = 202;
static constexpr int serverError = 500;
static constexpr int userError = 400;
static constexpr int badRequest = 400;
static constexpr int unauthorized = 401;
static constexpr int conflict = 409;
static constexpr int notFound = 404;
static constexpr int rateLimit = 429;
}  // namespace Config::StatusCodes

namespace Config::IssueMessages {
static constexpr const char *userNotFound = "User not founded";
static constexpr const char *invalidToken = "Invalid or expired token";
static constexpr const char *rateLimitExceed = "Rate limit exceeded";
}  // namespace Config::IssueMessages

#endif  // CODES_H
