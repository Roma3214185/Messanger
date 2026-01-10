#ifndef CACHEMIDDLEWARE_H
#define CACHEMIDDLEWARE_H

#include <crow.h>

#include <algorithm>

#include "MetricsMiddleware.h"
#include "interfaces/ICacheService.h"
#include "config/codes.h"

struct CacheMiddleware {
  ICacheService *cache_;
  struct context {
    std::optional<std::string> cached;
  };

  template <typename ParentCtx>
  void before_handle(const crow::request &req, crow::response &res, context &ctx, ParentCtx &parent_ctx) {
    if (req.method != crow::HTTPMethod::GET || notNeedCache(req.url)) return;
    LOG_INFO("Url before_handle cache: {}", req.url);

    auto key = makeCacheKey(req);
    auto val = cache_->get(key);
    if (!val) return;

    ctx.cached = val;
    res.code = Config::StatusCodes::success;
    res.write(val.value());
    auto &metrics_ctx = parent_ctx.template get<MetricsMiddleware>();
    metrics_ctx.hit_cache = true;
    LOG_INFO("Hit cache");
    res.end();
  }

  template <typename ParentCtx>
  void after_handle(const crow::request &req, crow::response &res, context &ctx, ParentCtx & /*unused*/) {
    if (req.method != crow::HTTPMethod::GET || notNeedCache(req.url)) return;
    auto key = makeCacheKey(req);
    cache_->set(key, res.body, std::chrono::seconds(30));
  }

 private:
  static std::string makeCacheKey(const crow::request &req) {
    // todo: implement for some url not set cache
    std::string key =
        "cache:" + crow::method_name(req.method) + ":" + req.url + (req.body.empty() ? "" : "|" + req.body);
    auto keys = req.url_params.keys();
    std::ranges::sort(keys);

    for (const auto& k : keys) {
      key += "|" + k + "=" + req.url_params.get(k);
    }

    if (!req.body.empty()) {
      key += "|body=" + req.body;
    }

    LOG_INFO("makeCacheKey for req {} : {} ", req.url, key);
    return key;
  }

  bool notNeedCache(const std::string& url) const {
    if (url == "/auth/me") return true;
    if (std::string req_url = "/request"; url.substr(0, req_url.length()) == req_url) return true;
    return false;
  }
};

#endif  // CACHEMIDDLEWARE_H
