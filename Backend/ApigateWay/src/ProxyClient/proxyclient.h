#ifndef APICLIENT_H
#define APICLIENT_H

#include <memory>

#include <httplib.h>
#include <crow/crow.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct ProxyClient
{
    std::string baseUrl;
    std::unique_ptr<httplib::Client> cli;
    std::string hostWithPort;

    ProxyClient(const std::string &url);

    std::pair<int, std::string> forward(const crow::request& req,
                                        const std::string& path, const std::string& method,
                                        const std::vector<std::pair<std::string,std::string>>& extra_headers = {});

    std::pair<int,std::string> post_json(const std::string& path, const json& body,
                                          const std::vector<std::pair<std::string,std::string>>& headers = {});
};

#endif // APICLIENT_H
