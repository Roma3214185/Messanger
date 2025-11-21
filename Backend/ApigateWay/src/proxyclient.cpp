#include "proxyclient.h"

using namespace std;

namespace {

std::string getContentType(const crow::request& req) {
  string content_type = req.get_header_value("content-type");
  if (content_type.empty()) content_type = "application/json";
  return content_type;
}

httplib::Result makeRequest(httplib::Client* client,
                            const std::string& method,
                            const std::string& full_path,
                            const httplib::Headers& headers,
                            const std::string& body = "",
                            const std::string& content_type = "application/json") {
  httplib::Result res(std::unique_ptr<httplib::Response>(nullptr), httplib::Error::Unknown);

  if (method == "GET") {
    res = client->Get(full_path.c_str(), headers);
  } else if (method == "DELETE") {
    res = client->Delete(full_path.c_str(), headers);
  } else if (method == "PUT") {
    res = client->Put(full_path.c_str(), headers, body, content_type.c_str());
  } else if (method == "POST") {
    res = client->Post(full_path.c_str(), headers, body, content_type.c_str());
  } else {
    res = httplib::Result(std::unique_ptr<httplib::Response>(nullptr), httplib::Error::Unknown);
  }

  return res;
}

}  // namespace


pair<int, string> ProxyClient::forward(const crow::request&                req,
                                       const RequestDTO& request_info,
                                       int port) {

  string host_with_port = "localhost:" + to_string(port);
  auto client_          = make_unique<httplib::Client>(host_with_port.c_str());
  client_->set_read_timeout(5, 0);
  client_->set_connection_timeout(5, 0);

  httplib::Headers headers;

  for (auto& h : req.headers) {
    headers.emplace(h.first, h.second);
  }

  for (auto& h : request_info.extra_headers) {
    headers.emplace(h.first, h.second);
  }

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

  std::string content_type = getContentType(req);
  auto res = makeRequest(client_.get(), request_info.method, full_path, headers, req.body, content_type);

  if (!res) {
    return {502, "Bad Gateway: downstream no response"};
  }

  return {static_cast<int>(res->status), res->body};
}
