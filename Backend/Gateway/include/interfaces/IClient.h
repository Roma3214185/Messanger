#ifndef IPROXYCLIENT_H
#define IPROXYCLIENT_H

#include <string>

#include "ForwardRequestDTO.h"

using ResponseCode = int;
using ResponseBody = std::string;
using NetworkResponse = std::pair<ResponseCode, ResponseBody>;

class IClient {
 public:
  virtual NetworkResponse Get(const ForwardRequestDTO &) = 0;
  virtual NetworkResponse Delete(const ForwardRequestDTO &) = 0;
  virtual NetworkResponse Put(const ForwardRequestDTO &) = 0;
  virtual NetworkResponse Post(const ForwardRequestDTO &) = 0;
  virtual ~IClient() = default;
};

#endif  // IPROXYCLIENT_H
