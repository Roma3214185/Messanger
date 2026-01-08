#ifndef RATELIMITMIDDLEWARE_H
#define RATELIMITMIDDLEWARE_H

#include <crow.h>

#include "ProdConfigProvider.h"
#include "interfaces/IRateLimiter.h"

struct RateLimitMiddleware {
  struct context {};
  IRateLimiter *rate_limiter_;
  IConfigProvider *provider_ = &ProdConfigProvider::instance();

  template <typename ParentCtx>
  void before_handle(const crow::request &req, crow::response &res,
                     context & /*ctx*/, ParentCtx & /*parent_ctx*/) {
    if (!rate_limiter_->allow(getIP(req))) {
      res.code = provider_->statusCodes().rateLimit;
      res.write(provider_->issueMessages().rateLimitExceed);
      res.end();
    }
  }

  template <typename ParentCtx>
  void after_handle(const crow::request & /*req*/, crow::response & /*res*/,
                    context & /*ctx*/, ParentCtx & /*unused*/) {
    // intentionally left empty
    // This middleware only needs to handle ratelimit before the main handler.
    // No post-processing is required after the request is handled.
  }

private:
  static std::string getIP(const crow::request &req) {
    auto ip = req.get_header_value("X-Forwarded-For");
    return ip.empty() ? req.remote_ip_address : ip;
  }
};

#endif // RATELIMITMIDDLEWARE_H
