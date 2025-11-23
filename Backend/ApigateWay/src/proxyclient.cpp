#include "proxyclient.h"

using namespace std;

constexpr int kBadGatewayCode = 502;
const string kBadGatewayMessage = "Bad Gateway: downstream no response";

namespace {

string getContentType(const crow::request& req) {
  string content_type = req.get_header_value("content-type");
  if (content_type.empty()) content_type = "application/json";
  return content_type;
}

string getFullPath(const crow::request& req, const RequestDTO& request_info) {
  string full_path = request_info.path;
  auto   keys      = req.url_params.keys();
  if (!keys.empty()) {
    full_path += "?";
    bool first = true;
    for (const auto& key : keys) {
      if (!first) full_path += "&";
      full_path += key;
      full_path += "=";
      full_path += req.url_params.get(key);
      first = false;
    }
  }
  return full_path;
}

httplib::Headers getHeaders(const crow::request& req, const RequestDTO& request_info) {
  httplib::Headers headers;
  for (auto& h : req.headers) {
    headers.emplace(h.first, h.second);
  }

  for (auto& h : request_info.extra_headers) {
    headers.emplace(h.first, h.second);
  }

  return headers;
}

std::string get_host_with_port(int port) {
  return "localhost:" + to_string(port);
}

}  // namespace

NetworkResponse ProxyClient::makeRequest(const ForwardRequestDTO& request, const std::string& method) {
  if (method == "GET") return client_->Get(request);
  if (method == "DELETE") return client_->Delete(request);
  if (method == "PUT") return client_->Put(request);
  if (method == "POST") return client_->Post(request);
  return {kBadGatewayCode, kBadGatewayMessage};
}

NetworkResponse ProxyClient::forward(const crow::request& req,
                                       RequestDTO& request_info,
                                       int port) {
  ForwardRequestDTO forward_request;
  forward_request.host_with_port = get_host_with_port(port); //TODO: make another DTO
  forward_request.headers = getHeaders(req, request_info);
  forward_request.body = req.body;
  forward_request.full_path = getFullPath(req, request_info);
  forward_request.content_type = getContentType(req);
  return makeRequest(forward_request, request_info.method);
}
