#ifndef PORTS_H
#define PORTS_H

struct Ports {
  static constexpr int authService = 8083;
  static constexpr int userService = 8083;
  static constexpr int chatService = 8081;
  static constexpr int notificationService = 8086;
  static constexpr int apigateService = 8084;
  static constexpr int messageService = 8082;
  static constexpr int rabitMQ = 5672;
  static constexpr int metrics = 8089;
};

#endif // PORTS_H
