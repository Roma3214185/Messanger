#ifndef REQUESTDTO_H
#define REQUESTDTO_H

#include <string>
#include <vector>
#include <unordered_map>
#include <crow/crow.h>
#include <nlohmann/json.hpp>

struct RequestDTO {
    std::string path;
    std::string method;
    std::string body;
    std::string request_id;
    std::string content_type;
    std::vector<std::pair<std::string, std::string>> headers;
    std::multimap<std::string, std::string> url_params;
};

namespace utils {

inline std::string getContentType(const crow::request& req) {
  std::string content_type = req.get_header_value("content-type");
  return content_type.empty() ? "application/json" : content_type;
}

inline RequestDTO getDTO(const crow::request& req, const std::string& path, const int request_id = -1) {
    RequestDTO request_info;
    request_info.method = crow::method_name(req.method);
    request_info.path = path;
    request_info.body = req.body;
    request_info.content_type = getContentType(req);
    request_info.request_id = request_id;

    auto keys = req.url_params.keys();
    for (const std::string& key : keys) {
      request_info.url_params.emplace(key, req.url_params.get(key));
    }

    for (auto const& header : req.headers) {
      request_info.headers.emplace_back(header.first, header.second);
    }

    return request_info;
  }

}  // utils

namespace nlohmann {

template <>
struct adl_serializer<RequestDTO> {
  static void to_json(json& j, const RequestDTO& r) {
    j = json{
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
