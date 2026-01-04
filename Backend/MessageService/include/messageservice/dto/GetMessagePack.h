#ifndef GETMESSAGEPACK_H
#define GETMESSAGEPACK_H

struct GetMessagePack {
  long long chat_id;
  int       limit;
  int       before_id;
  long long user_id;
};

#endif  // GETMESSAGEPACK_H
