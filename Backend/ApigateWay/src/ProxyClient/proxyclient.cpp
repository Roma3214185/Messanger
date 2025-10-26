#include "proxyclient.h"

using namespace std;

ProxyClient::ProxyClient(const string &url)
    : baseUrl(url)
{
    string host = url;
    if (host.rfind("http://", 0) == 0) host = host.substr(7);
    else if (host.rfind("https://", 0) == 0) host = host.substr(8);

    hostWithPort = host;
    cli = make_unique<httplib::Client>(host.c_str());
    cli->set_read_timeout(5,0);
    cli->set_connection_timeout(5,0);
}

pair<int, string> ProxyClient::post_json(const string& path, const json& body, const vector<pair<string,string>>& headers) {
    auto s = body.dump();
    httplib::Headers h;
    h.emplace("Content-Type", "application/json");
    for (auto &p : headers) h.emplace(p.first, p.second);
    auto res = cli->Post(path.c_str(), h, s, "application/json");
    if (!res) return {502, "Bad Gateway"};
    return {(int)res->status, res->body};
}

pair<int, string> ProxyClient::forward(const crow::request& req, const string& path, const string& method, const vector<pair<string,string>>& extra_headers) {
    httplib::Headers headers;
    for (auto &h : req.headers) {
        headers.emplace(h.first, h.second);
    }

    for (auto &h : extra_headers) {
        headers.emplace(h.first, h.second);
    }

    httplib::Result res(unique_ptr<httplib::Response>(nullptr), httplib::Error::Unknown);

    cout << "METHOD: " << method << endl;
    cout << "PATH: " << path.c_str() << endl;

    if (method == "GET") res = cli->Get(path.c_str(), headers);
    else if (method == "DELETE") res = cli->Delete(path.c_str(), headers);
    else if (method == "PUT") res = cli->Put(path.c_str(), headers, req.body, req.get_header_value("content-type").c_str());
    else res = cli->Post(path.c_str(), headers, req.body, req.get_header_value("content-type").c_str());

    if (!res) return {502, "Bad Gateway: downstream no response"};
    return { (int)res->status, res->body };
}
