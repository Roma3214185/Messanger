#ifndef MOCKSOCKET_H
#define MOCKSOCKET_H

#include "interfaces/ISocket.h"

class MockSocket : public ISocket {
public:
  void send_text(const std::string &text) override {
    ++send_text_calls;
    last_sended_text = text;
  }

  int send_text_calls = 0;
  std::string last_sended_text = "";
};

#endif // MOCKSOCKET_H
