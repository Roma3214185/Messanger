#ifndef SOCKETNOTIFIER_H
#define SOCKETNOTIFIER_H

#include <nlohmann/json.hpp>  //todo: refactor to send only std::string

class IUserSocketRepository;

class INotifier {
 public:
  virtual ~INotifier() = default;
  virtual bool notifyMember(long long user_id, nlohmann::json json_message, std::string type) = 0;
};

class SocketNotifier : public INotifier {
 public:
  SocketNotifier(IUserSocketRepository* sock_manager);
  bool notifyMember(long long user_id, nlohmann::json json_message, std::string type);

 private:
  IUserSocketRepository* socket_manager_;
};

#endif  // SOCKETNOTIFIER_H
