#ifndef AUTORITIZERPROVIDER_H
#define AUTORITIZERPROVIDER_H

#include <memory>
#include <optional>

#include "chatservice/TokenService.h"
#include "interfaces/IAutoritizer.h"

class RealAutoritizer : public IAutoritizer {
public:
  std::optional<long long> autoritize(const std::string &token) override {
    return JwtUtils::verifyTokenAndGetUserId(token);
  }
};

class AutoritizerProvider { // todo: delete AutoritizerProvider class
public:
  static void set(std::shared_ptr<IAutoritizer> a) {
    instance() = std::move(a);
  }

  static std::shared_ptr<IAutoritizer> get() { return instance(); }

private:
  static std::shared_ptr<IAutoritizer> &instance() {
    static std::shared_ptr<IAutoritizer> inst =
        std::make_shared<RealAutoritizer>();
    return inst;
  }
};

#endif // AUTORITIZERPROVIDER_H
