#ifndef BACKEND_CHATSERVICE_SRC_HEADERS_CHAT_H_
#define BACKEND_CHATSERVICE_SRC_HEADERS_CHAT_H_

#include <QDateTime>
#include <string>

struct Chat {
  int id;
  bool isGroup;
  std::string name;
  std::string avatar;
  QDateTime createdAt;
};

#endif  // BACKEND_CHATSERVICE_SRC_HEADERS_CHAT_H_
