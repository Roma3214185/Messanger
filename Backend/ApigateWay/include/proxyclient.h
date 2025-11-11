#ifndef BACKEND_APIGATEWAY_SRC_PROXYCLIENT_PROXYCLIENT_H_
#define BACKEND_APIGATEWAY_SRC_PROXYCLIENT_PROXYCLIENT_H_

#include <crow.h>
#include <httplib.h>

#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct ProxyClient {
  std::string                      baseUrl;
  std::unique_ptr<httplib::Client> cli;
  std::string                      hostWithPort;

  ProxyClient(const std::string& url);

  std::pair<int, std::string> forward(
      const crow::request&                                    req,
      const std::string&                                      path,
      const std::string&                                      method,
      const std::vector<std::pair<std::string, std::string>>& extra_headers = {});

  std::pair<int, std::string> post_json(
      const std::string&                                      path,
      const json&                                             body,
      const std::vector<std::pair<std::string, std::string>>& headers = {});
};

#endif  // BACKEND_APIGATEWAY_SRC_PROXYCLIENT_PROXYCLIENT_H_
