#ifndef ISOCKETRESPONCEHANDLER_H
#define ISOCKETRESPONCEHANDLER_H

#include <QJsonObject>

class ISocketResponceHandler {
 public:
  virtual ~ISocketResponceHandler()                   = default;
  virtual void handle(const QJsonObject& json_object) = 0;
};

#endif  // ISOCKETRESPONCEHANDLER_H
