#include "Logger.h"

#include <chrono>

#include "Debug_profiling.h"
#include "nlohmann/json.hpp"

void Logger::logRequest(const crow::request& req,
                               const RequestDTO& request_info)
{
  nlohmann::json log;

  log["type"] = "request";
  log["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count();

  log["method"] = request_info.method;
  log["path"] = request_info.path;
  log["ip"] = req.remote_ip_address;

  nlohmann::json queryJson = nlohmann::json::object();
  for (const auto& key : req.url_params.keys()) {
    auto value = req.url_params.get(key);
    if (value)
      queryJson[key] = *value;
  }
  log["query"] = queryJson;

  // Skip sensitive headers
  nlohmann::json headers = nlohmann::json::object();
  for (auto& h : req.headers) {
    if (h.first == "Authorization") continue;
    if (h.first == "Cookie") continue;
    headers[h.first] = h.second;
  }
  log["headers"] = headers;

  if (request_info.method == "GET" || request_info.method == "DELETE") {
    log["body"] = nullptr;
  } else {
    if (req.body.size() < 2000)
      log["body"] = req.body;
    else
      log["body"] = "[body omitted: too large]";
  }

  LOG_INFO("REQ: {}", log.dump());
}

void Logger::logResponse(int status,
                                const std::string& body,
                                const RequestDTO& request_info,
                                bool cacheHit)
{
  nlohmann::json log;

  log["type"] = "response";
  log["timestamp"] = std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count();

  log["method"] = request_info.method;
  log["path"] = request_info.path;
  log["status"] = status;
  log["cache_hit"] = cacheHit;

  if (body.size() <= 2000)
    log["body"] = body;
  else
    log["body"] = "[response omitted: too large]";

  LOG_INFO("RES: {}", log.dump());
}
