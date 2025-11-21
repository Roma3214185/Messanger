#ifndef BACKEND_APIGATEWAY_SRC_PROXYCLIENT_PROXYCLIENT_H_
#define BACKEND_APIGATEWAY_SRC_PROXYCLIENT_PROXYCLIENT_H_

#include <crow.h>
#include <httplib.h>

#include <memory>
#include <nlohmann/json.hpp>

struct ProxyClient {
  int                     port_;
  std::unique_ptr<httplib::Client> client_;

  ProxyClient(int port);

  std::pair<int, std::string> forward(
      const crow::request&                                    req,
      const std::string&                                      path,
      const std::string&                                      method,
      const std::vector<std::pair<std::string, std::string>>& extra_headers = {});

  std::pair<int, std::string> post_json(
      const std::string&                                      path,
      const nlohmann::json&                                   body,
      const std::vector<std::pair<std::string, std::string>>& headers = {});
};

#endif  // BACKEND_APIGATEWAY_SRC_PROXYCLIENT_PROXYCLIENT_H_
