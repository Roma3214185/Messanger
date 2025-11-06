#include "proxyclient.h"

using namespace std;

ProxyClient::ProxyClient(const string& url) : baseUrl(url) {
  string host = url;
  if (host.rfind("http://", 0) == 0)
    host = host.substr(7);
  else if (host.rfind("https://", 0) == 0)
    host = host.substr(8);

  hostWithPort = host;
  cli = make_unique<httplib::Client>(host.c_str());
  cli->set_read_timeout(5, 0);
  cli->set_connection_timeout(5, 0);
}

pair<int, string> ProxyClient::post_json(
    const string& path, const json& body,
    const vector<pair<string, string>>& headers) {
  auto s = body.dump();
  httplib::Headers h;
  h.emplace("Content-Type", "application/json");
  for (auto& p : headers) h.emplace(p.first, p.second);
  auto res = cli->Post(path.c_str(), h, s, "application/json");
  if (!res) return {502, "Bad Gateway"};
  return {(int)res->status, res->body};
}

pair<int, string> ProxyClient::forward(
    const crow::request& req, const string& path, const string& method,
    const vector<pair<string, string>>& extra_headers) {

  httplib::Headers headers;

  // Передаємо всі заголовки від клієнта
  for (auto& h : req.headers) {
    headers.emplace(h.first, h.second);
  }

  // Додаємо додаткові заголовки
  for (auto& h : extra_headers) {
    headers.emplace(h.first, h.second);
  }

  string full_path = path;
  auto keys = req.url_params.keys();
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
  if (content_type.empty()) content_type = "application/json"; // або default

  httplib::Result res(std::unique_ptr<httplib::Response>(nullptr),
                      httplib::Error::Unknown);
  if (method == "GET")
    res = cli->Get(full_path.c_str(), headers);
  else if (method == "DELETE")
    res = cli->Delete(full_path.c_str(), headers);
  else if (method == "PUT")
    res = cli->Put(full_path.c_str(), headers, req.body, content_type.c_str());
  else // POST
    res = cli->Post(full_path.c_str(), headers, req.body, content_type.c_str());

  if (!res) {
    return {502, "Bad Gateway: downstream no response"};
  }

  return {static_cast<int>(res->status), res->body};
}

