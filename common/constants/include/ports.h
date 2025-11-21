#ifndef PORTS_H
#define PORTS_H

struct Ports {
  int authService         = 8083;
  int userService         = 8083;
  int chatService         = 8081;
  int notificationService = 8086;
  int apigateService      = 8084;
  int messageService      = 8082;
  int rabitMQ             = 5672;
  int metrics             = 8089;
};

#endif // PORTS_H
