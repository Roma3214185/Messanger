#ifndef CACHEMIDDLEWARE_H
#define CACHEMIDDLEWARE_H

#include <crow.h>
#include "interfaces/ICacheService.h"
#include "ProdConfigProvider.h"
#include "MetricsMiddleware.h"

struct CacheMiddleware {
    ICacheService* cache_;
    IConfigProvider* provider = &ProdConfigProvider::instance();
    struct context { std::optional<std::string> cached; };

    template<typename ParentCtx>
    void before_handle(const crow::request& req,
                       crow::response& res,
                       context& ctx,
                       ParentCtx& parent_ctx)
    {
      if(req.method != crow::HTTPMethod::GET) return;
      auto key = makeCacheKey(req);
      if(auto val = cache_->get(key)) {
        ctx.cached = val;
        res.code = provider->statusCodes().success;
        res.write(val.value());
        auto& metrics_ctx = parent_ctx.template get<MetricsMiddleware>();
        metrics_ctx.hit_cache = true;
        res.end();
      }
    }

    template<typename ParentCtx>
    void after_handle(const crow::request& req, crow::response& res, context& ctx, ParentCtx&) {
      if(req.method != crow::HTTPMethod::GET) return;
      auto key = makeCacheKey(req);
      cache_->set(key, res.body);
    }

  private:
    std::string makeCacheKey(const crow::request& req) {
      //TODO: think about valid of this key
      return "cache:" + crow::method_name(req.method) + ":" + req.url + (req.body.empty() ? "" : "|" + req.body);
    }
};


#endif // CACHEMIDDLEWARE_H
