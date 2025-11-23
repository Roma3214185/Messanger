#ifndef CACHEMIDDLEWARE_H
#define CACHEMIDDLEWARE_H

#include <crow.h>
//#include "interfaces/ICache.h"
#include "interfaces/ICacheService.h"

struct CacheMiddleware {
    ICacheService* cache_;
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
        res.write(val.value());
        res.end();
      }
    }

    template<typename ParentCtx>
    void after_handle(const crow::request& req, crow::response& res, context& ctx, ParentCtx&) {
      auto key = makeCacheKey(req);
      cache_->set(key, res.body);
    }

  private:
    std::string makeCacheKey(const crow::request& req) {
      return "cache:" + getMethod(req.method) + ":" + req.url;
    }

    std::string getMethod(const crow::HTTPMethod& method) {
      switch (method) {
      case crow::HTTPMethod::GET:
        return "GET";
      case crow::HTTPMethod::Delete:
        return "DELETE";
      case crow::HTTPMethod::Put:
        return "PUT";
      default:
        return "POST";
      }
    }
};


#endif // CACHEMIDDLEWARE_H
