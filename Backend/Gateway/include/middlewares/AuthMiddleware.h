#ifndef AUTHMIDDLEWARE_H
#define AUTHMIDDLEWARE_H

#include <crow.h>

#include <algorithm>

#include "Debug_profiling.h"
#include "config/codes.h"
#include "interfaces/IVerifier.h"

struct AuthMiddleware {
  struct context {
    long long user_id = -1;
  } cont;
  IVerifier *verifier_;

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

    res.code = Config::StatusCodes::unauthorized;
    res.write(Config::IssueMessages::invalidToken);
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
  }
};

#endif  // AUTHMIDDLEWARE_H
