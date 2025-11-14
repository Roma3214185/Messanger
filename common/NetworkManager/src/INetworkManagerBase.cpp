#include "interfaces/INetworkManagerBase.h"

#include <httplib.h>

using std::pair;
using std::string;
using std::vector;
using std::unique_ptr;

pair<int, string> INetworkManagerBase::forward(
                          int                                 port,
                          const string&                       body,
                          const string&                       path,
                          const string&                       method,
                          const vector<pair<string, string>>& extra_headers) {
  httplib::Headers headers;
  for (const auto& h : extra_headers) {
    headers.emplace(h.first, h.second);
  }

  httplib::Client cli("localhost", port);
  cli.set_connection_timeout(5, 0);

  httplib::Result res(unique_ptr<httplib::Response>(nullptr), httplib::Error::Unknown);

  if (method == "GET") {
    res = cli.Get(path.c_str(), headers);
  } else if (method == "DELETE") {
    res = cli.Delete(path.c_str(), headers);
  } else if (method == "PUT") {
    res = cli.Put(path.c_str(), headers, body, "application/json");
  } else {
    res = cli.Post(path.c_str(), headers, body, "application/json");
  }

  if (!res) {
    return {provider_->statusCodes().serverError, "Bad Gateway: downstream no response"};
  }

  return {res->status, res->body};
}
