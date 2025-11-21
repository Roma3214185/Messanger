#include "proxyclient.h"

using namespace std;

ProxyClient::ProxyClient(int port) : port_(port) {
  string host_with_port = "localhost:" + to_string(port);
  client_          = make_unique<httplib::Client>(host_with_port.c_str());
  client_->set_read_timeout(5, 0);
  client_->set_connection_timeout(5, 0);
}

pair<int, string> ProxyClient::post_json(const string&                       path,
                                         const nlohmann::json&                         body,
                                         const vector<pair<string, string>>& headers) {
  auto             s = body.dump();
  httplib::Headers h;
  h.emplace("Content-Type", "application/json");
  for (auto& p : headers) h.emplace(p.first, p.second);
  auto res = client_->Post(path.c_str(), h, s, "application/json");
  if (!res) return {502, "Bad Gateway"};
  return {(int)res->status, res->body};
}

pair<int, string> ProxyClient::forward(const crow::request&                req,
                                       const string&                       path,
                                       const string&                       method,
                                       const vector<pair<string, string>>& extra_headers) {
  httplib::Headers headers;

  for (auto& h : req.headers) {
    headers.emplace(h.first, h.second);
  }

  for (auto& h : extra_headers) {
    headers.emplace(h.first, h.second);
  }

  string full_path = path;
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

  cout << "METHOD: " << method << endl;
  cout << "PATH: " << full_path << endl;

  string content_type = req.get_header_value("content-type");
  if (content_type.empty()) content_type = "application/json";  // або default

  httplib::Result res(std::unique_ptr<httplib::Response>(nullptr), httplib::Error::Unknown);
  if (method == "GET") {
    res = client_->Get(full_path.c_str(), headers);
  } else if (method == "DELETE") {
    res = client_->Delete(full_path.c_str(), headers);
  } else if (method == "PUT") {
    res = client_->Put(full_path.c_str(), headers, req.body, content_type.c_str());
  } else {
    res = client_->Post(full_path.c_str(), headers, req.body, content_type.c_str());
  }

  if (!res) {
    return {502, "Bad Gateway: downstream no response"};
  }

  return {static_cast<int>(res->status), res->body};
}
