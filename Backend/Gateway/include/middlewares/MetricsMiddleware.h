#ifndef METRICSMIDDLEWARE_H
#define METRICSMIDDLEWARE_H

#include <crow.h>

#include "Debug_profiling.h"
#include "MetricsTracker.h"
#include "interfaces/IMetrics.h"

struct MetricsMiddleware {
  struct context {
    MetricsTracker tracker;
    bool hit_cache = false;
  };
  IMetrics *metrics_;

  template <typename ParentCtx>
  void before_handle(const crow::request &req, crow::response & /*res*/, context &ctx, ParentCtx & /*parent_ctx*/) {
    assert(metrics_ != nullptr);
    ctx.tracker.startTimer(metrics_);
    metrics_->newRequest(req.url);
    // TODO: for /ws??
  }

  template <typename ParentCtx>
  void after_handle(const crow::request &req, crow::response &res, context &ctx, ParentCtx & /*unused*/) {
    // if(req.url == "\ws") return; //TODO: think about user
    // connected/disconnected statuses
    metrics_->requestEnded(crow::method_name(req.method), res.code, ctx.hit_cache);
  }
};

#endif  // METRICSMIDDLEWARE_H
