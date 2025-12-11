#ifndef REQUESTDTO_H
#define REQUESTDTO_H

#include <string>
#include <vector>

struct RequestDTO{
    std::string path;
    std::string method;
    std::string request_id;
    std::vector<std::pair<std::string, std::string>> extra_headers = {};
};

#endif // REQUESTDTO_H
