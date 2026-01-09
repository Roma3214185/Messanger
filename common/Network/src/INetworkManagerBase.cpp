#include "interfaces/INetworkManagerBase.h"

#include <httplib.h>

#include "Debug_profiling.h"

using std::pair;
using std::string;
using std::unique_ptr;
using std::vector;

pair<int, string> INetworkManagerBase::forward(int port, const string &body,
                                               const string &path,  // todo: strong_type???
                                               const string &method,
                                               const vector<pair<string, string>> &extra_headers) {
  httplib::Headers headers;
  for (const auto &h : extra_headers) {
    headers.emplace(h.first, h.second);
  }

  httplib::Client cli("localhost", port);
  cli.set_connection_timeout(5, 0);  // todo: gateway calss to forward message -> here

  auto res = [&]() {
    LOG_INFO("Path in forward {}", path);
    if (method == "GET") return cli.Get(path, headers);
    if (method == "DELETE") return cli.Delete(path, headers);
    if (method == "PUT") return cli.Put(path, headers, body, "application/json");
    if (method == "POST") return cli.Post(path, headers, body, "application/json");

    return httplib::Result(unique_ptr<httplib::Response>(nullptr), httplib::Error::Unknown);
  }();

  if (!res) {
    return {provider_->statusCodes().serverError, "Bad Gateway: downstream no response"};
  }

  return {res->status, res->body};
}
