#ifndef AUTHMIDDLEWARE_H
#define AUTHMIDDLEWARE_H

#include <crow.h>

#include <algorithm>

#include "Debug_profiling.h"
#include "ProdConfigProvider.h"
#include "interfaces/IConfigProvider.h"
#include "interfaces/IVerifier.h"

struct AuthMiddleware {
  struct context {
    long long user_id = -1;
  } cont;
  IVerifier *verifier_;
  IConfigProvider *provider = &ProdConfigProvider::instance();

  template <typename ParentCtx>
  void before_handle(const crow::request &req, crow::response &res, context &ctx, ParentCtx &parent_ctx) {
    if (!needToAuth(req.url)) return;

    auto token = fetchToken(req);
    std::optional<long long> id = verifier_->verifyTokenAndGetUserId(token);
    if (id.has_value()) {
      cont.user_id = *id;
      return;
    }

    LOG_INFO("Unautoritized {}", req.url);

    res.code = provider->statusCodes().unauthorized;
    res.write(provider->issueMessages().invalidToken);
    res.end();
  }

  template <typename ParentCtx>
  void after_handle(const crow::request &req, crow::response &res, context &ctx, ParentCtx &) {
    // intentionally left empty
    // This middleware only needs to handle authentication before the main handler.
    // No post-processing is required after the request is handled.
  }

 private:
  inline static const std::vector<std::string> kNoNeedAuthUrls{"/auth/login", "/auth/register", "/ws", "/request"};

  inline static std::string fetchToken(const crow::request &req) { return req.get_header_value("Authorization"); }

  inline static bool needToAuth(const std::string &url) {
    LOG_INFO("Check to auth url {}", url);
    return std::none_of(kNoNeedAuthUrls.begin(), kNoNeedAuthUrls.end(),
                        [&](const auto &prefix) { return url.starts_with(prefix); });

    // for(const auto &need_auth_url : kNoNeedAuthUrls) {
    //   if(url.substr(0, need_auth_url.length()) == need_auth_url) return
    //   false;
    // }
    // return true;
  }
};

#endif  // AUTHMIDDLEWARE_H
