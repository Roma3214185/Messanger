#include "proxyclient.h"

ProxyClient::ProxyClient(const std::string &url) : baseUrl(url) {
    // naive parse: remove scheme
    std::string host = url;
    if (host.rfind("http://", 0) == 0) host = host.substr(7);
    else if (host.rfind("https://", 0) == 0) host = host.substr(8);
    // store hostWithPort
    hostWithPort = host;
    cli = std::make_unique<httplib::Client>(host.c_str());
    cli->set_read_timeout(5,0);
    cli->set_connection_timeout(5,0);
}


std::pair<int,std::string> ProxyClient::post_json(const std::string& path, const json& body, const std::vector<std::pair<std::string,std::string>>& headers) {
    auto s = body.dump();
    httplib::Headers h;
    h.emplace("Content-Type", "application/json");
    for (auto &p : headers) h.emplace(p.first, p.second);
    auto res = cli->Post(path.c_str(), h, s, "application/json");
    if (!res) return {502, "Bad Gateway"};
    return {(int)res->status, res->body};
}

std::pair<int, std::string> ProxyClient::forward(const crow::request& req, const std::string& path, const std::string& method, const std::vector<std::pair<std::string,std::string>>& extra_headers) {
    httplib::Headers headers;
    for (auto &h : req.headers) {
        headers.emplace(h.first, h.second);
    }
    for (auto &h : extra_headers) headers.emplace(h.first, h.second);

    httplib::Result res(std::unique_ptr<httplib::Response>(nullptr), httplib::Error::Unknown);

    std::cout << "METHOD: " << method << std::endl;
    std::cout << "PATH: " << path.c_str() << std::endl;

    if (method == "GET") res = cli->Get(path.c_str(), headers);
    else if (method == "DELETE") res = cli->Delete(path.c_str(), headers);
    else if (method == "PUT") res = cli->Put(path.c_str(), headers, req.body, req.get_header_value("content-type").c_str());
    else res = cli->Post(path.c_str(), headers, req.body, req.get_header_value("content-type").c_str());

    if (!res) return {502, "Bad Gateway: downstream no response"};
    return { (int)res->status, res->body };
}
