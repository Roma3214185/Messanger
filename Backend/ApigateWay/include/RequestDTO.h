#ifndef REQUESTDTO_H
#define REQUESTDTO_H

#include <string>

struct RequestDTO{
    // int request_id
    std::string path;
    std::string method;
};

#endif // REQUESTDTO_H
