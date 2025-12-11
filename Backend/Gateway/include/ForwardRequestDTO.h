#ifndef FORWARDREQUESTDTO_H
#define FORWARDREQUESTDTO_H

#include <vector>
#include <chrono>
#include <httplib.h>

struct ForwardRequestDTO{
    std::string host_with_port;
    std::string full_path;
    httplib::Headers headers;
    std::vector<std::pair<std::string, std::string>> extra_headers;
    std::string body = "";
    std::string content_type = "application/json";
    int times_retrying = 3;
    std::chrono::milliseconds timeout{2000};
};

#endif // FORWARDREQUESTDTO_H
