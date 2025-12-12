#ifndef REQUESTDTO_H
#define REQUESTDTO_H

#include <string>
#include <vector>
#include <unordered_map>

#include "crow/crow.h"

struct RequestDTO{
    std::string path;
    std::string method;
    std::string body;
    std::string request_id;
    std::string content_type;
    std::vector<std::pair<std::string, std::string>> headers;
    std::unordered_map<std::string, std::string> url_params;
};

#endif // REQUESTDTO_H
