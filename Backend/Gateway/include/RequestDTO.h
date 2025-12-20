#ifndef REQUESTDTO_H
#define REQUESTDTO_H

#include <string>
#include <vector>
#include <unordered_map>

#include <nlohmann/json.hpp>

struct RequestDTO {
    int port;
    std::string path;
    std::string method;
    std::string body;
    std::string request_id;
    std::string content_type;
    std::vector<std::pair<std::string, std::string>> headers;
    std::multimap<std::string, std::string> url_params;
};

namespace nlohmann {

template <>
struct adl_serializer<RequestDTO> {
  static void to_json(json& j, const RequestDTO& r) {
    j = json{
        {"port", r.port},
        {"path", r.path},
        {"method", r.method},
        {"body", r.body},
        {"request_id", r.request_id},
        {"content_type", r.content_type},
        {"headers", r.headers},
        {"url_params", r.url_params}
    };
  }

  static void from_json(const json& j, RequestDTO& r) {
    j.at("port").get_to(r.port);
    j.at("path").get_to(r.path);
    j.at("method").get_to(r.method);
    j.at("body").get_to(r.body);
    j.at("request_id").get_to(r.request_id);
    j.at("content_type").get_to(r.content_type);
    j.at("headers").get_to(r.headers);
    j.at("url_params").get_to(r.url_params);
  }
};

} // namespace nlohmann

#endif // REQUESTDTO_H
