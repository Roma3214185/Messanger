#ifndef ISOCKET_H
#define ISOCKET_H

#include <string>

class ISocket {
 public:
  virtual void send_text(const std::string &text) = 0;
  virtual ~ISocket() = default;
};

#endif  // ISOCKET_H
