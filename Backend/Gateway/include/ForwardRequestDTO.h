#ifndef FORWARDREQUESTDTO_H
#define FORWARDREQUESTDTO_H

#include <httplib.h>

#include <chrono>
#include <vector>

struct ForwardRequestDTO {
  std::string host_with_port;
  std::string full_path;
  httplib::Headers headers;
  httplib::Params params;
  std::vector<std::pair<std::string, std::string>> extra_headers;
  std::string body;
  std::string content_type{"application/json"};
  int times_retrying{3};
  std::chrono::milliseconds timeout{2000};
};

#endif // FORWARDREQUESTDTO_H
