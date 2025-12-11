#ifndef AUTHMIDDLEWARE_H
#define AUTHMIDDLEWARE_H

#include <crow.h>

#include "interfaces/IVerifier.h"
#include "Debug_profiling.h"
#include "interfaces/IConfigProvider.h"
#include "ProdConfigProvider.h"

struct AuthMiddleware {
    struct context {};
    IVerifier* verifier_;
    IConfigProvider* provider = &ProdConfigProvider::instance();

    template<typename ParentCtx>
    void before_handle(const crow::request& req,
                       crow::response& res,
                       context& ctx,
                       ParentCtx& parent_ctx) {
      if(!needToAuth(req.url)) return;

      auto token = fetchToken(req);
      if(verifier_->verifyTokenAndGetUserId(token)) {
        return;
      }

      LOG_INFO("Unautoritized {}", req.url);

      res.code = provider->statusCodes().unauthorized;
      res.write(provider->issueMessages().invalidToken);
      res.end();
    }

    template<typename ParentCtx>
    void after_handle(const crow::request& req, crow::response& res, context& ctx, ParentCtx&) {

    }

  private:
    inline static const std::vector<std::string> no_need_auth_urls {"/auth/login", "/auth/register", "/ws", "/request"};

    std::string fetchToken(const crow::request& req) {
      return req.get_header_value("Authorization");
    }

    bool needToAuth(const std::string& url) {
      LOG_INFO("Check to auth url {}", url);
      for(const auto &need_auth_url : no_need_auth_urls) {
        if(url.substr(0, need_auth_url.length()) == need_auth_url) return false;
      }
      return true;
    }
};

#endif // AUTHMIDDLEWARE_H
