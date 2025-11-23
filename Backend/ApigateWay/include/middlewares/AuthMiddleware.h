#ifndef AUTHMIDDLEWARE_H
#define AUTHMIDDLEWARE_H

#include <crow.h>

#include "interfaces/IVerifier.h"
#include "Debug_profiling.h"

struct AuthMiddleware {
    struct context {};
    IVerifier* verifier_;

    template<typename ParentCtx>
    void before_handle(const crow::request& req,
                       crow::response& res,
                       context& ctx,
                       ParentCtx& parent_ctx) {
      if(req.url == "/auth/login" || req.url == "/auth/register" || req.url == "/ws") {
        return;
      }

      auto token = fetchToken(req);
      if(!token.empty() && verifier_->verifyTokenAndGetUserId(token)) {
        return;
      }

      LOG_INFO("Unautoritized {}", req.url);

      res.code = 401;
      res.write("Unauthorized");
      res.end();
    }

    template<typename ParentCtx>
    void after_handle(const crow::request& req, crow::response& res, context& ctx, ParentCtx&) {

    }

  private:
    std::string fetchToken(const crow::request& req) {
      return req.get_header_value("Authorization");
    }
};

#endif // AUTHMIDDLEWARE_H
