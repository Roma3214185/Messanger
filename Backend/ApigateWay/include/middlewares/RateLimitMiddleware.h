#ifndef RATELIMITMIDDLEWARE_H
#define RATELIMITMIDDLEWARE_H

#include <crow.h>

#include "interfaces/IRateLimiter.h"
#include "ProdConfigProvider.h"

struct RateLimitMiddleware {
    IRateLimiter* rate_limiter_;
    IConfigProvider* provider_ = &ProdConfigProvider::instance();
    struct context {};

    template<typename ParentCtx>
    void before_handle(const crow::request& req,
                       crow::response& res,
                       context& ctx,
                       ParentCtx& parent_ctx) {
      if(!rate_limiter_->allow(getIP(req))) {
        res.code = provider_->statusCodes().rateLimit;
        res.write(provider_->issueMessages().rateLimitExceed);
        res.end();
      }
    }

    template<typename ParentCtx>
    void after_handle(const crow::request& req, crow::response& res, context& ctx, ParentCtx&) {

    }

  private:
    std::string getIP(const crow::request& req) const {
      auto ip = req.get_header_value("X-Forwarded-For");
      if (ip.empty()) {
        ip = req.remote_ip_address;
      }
      return ip;
    }
};

#endif // RATELIMITMIDDLEWARE_H
