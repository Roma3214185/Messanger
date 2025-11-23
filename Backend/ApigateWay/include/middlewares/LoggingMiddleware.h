#ifndef LOGGINGMIDDLEWARE_H
#define LOGGINGMIDDLEWARE_H

#include <crow.h>
#include <nlohmann/json.hpp>

#include "Debug_profiling.h"

struct LoggingMiddleware {
    struct context {
        std::chrono::steady_clock::time_point start_time;
        std::string request_id;
    };

    inline static std::atomic<uint64_t> global_request_counter{0};

    std::string generateRequestId() {
      uint64_t id = global_request_counter.fetch_add(1, std::memory_order_relaxed);
      std::stringstream ss;
      ss << "req-" << std::setw(6) << std::setfill('0') << id;
      return ss.str();
    }

    template<typename ParentCtx>
    void before_handle(const crow::request& req,
                       crow::response& res,
                       context& ctx,
                       ParentCtx& parent_ctx) {
      ctx.start_time = std::chrono::steady_clock::now();
      ctx.request_id = generateRequestId();

      nlohmann::json json;
      json["request_id"] = ctx.request_id;
      json["event"] = "request_received";
      json["method"] = getMethod(req.method);
      json["url"] = req.url;
      json["client_ip"] = req.remote_ip_address;

      LOG_INFO("REQ: {}", json.dump());
    }

    template<typename ParentCtx>
    void after_handle(const crow::request& req, crow::response& res, context& ctx, ParentCtx&) {
      auto duration = std::chrono::steady_clock::now() - ctx.start_time;
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

      nlohmann::json json;
      json["request_id"] = ctx.request_id;
      json["event"] = "response_sent";
      json["method"] = getMethod(req.method);
      json["url"] = req.url;
      json["client_ip"] = req.remote_ip_address;
      json["duration_ms"] = ms;

      LOG_INFO("RES: {}", json.dump());
    }

  private:
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

#endif // LOGGINGMIDDLEWARE_H
