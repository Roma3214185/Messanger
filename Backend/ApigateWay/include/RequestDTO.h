#ifndef REQUESTDTO_H
#define REQUESTDTO_H

#include <string>
#include <vector>

struct RequestDTO{
    // int request_id
    std::string path;
    std::string method;
    std::vector<std::pair<std::string, std::string>> extra_headers = {};
};

#endif // REQUESTDTO_H
