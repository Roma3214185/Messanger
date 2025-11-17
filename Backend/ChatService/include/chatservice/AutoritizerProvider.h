#ifndef AUTORITIZERPROVIDER_H
#define AUTORITIZERPROVIDER_H

#include <memory>
#include <optional>

#include "chatservice/TokenService.h"

class IAutoritizer {
  public:
    virtual ~IAutoritizer() = default;
    virtual std::optional<int> autoritize(const std::string& token) = 0;
};

class RealAutoritizer : public IAutoritizer {
  public:
    std::optional<int> autoritize(const std::string& token) override {
      return JwtUtils::verifyTokenAndGetUserId(token);
    }
};

class MockAutoritizer : public IAutoritizer {
  public:
    std::optional<int> mock_user_id;
    std::string last_token;
    int call_autoritize = 0;
    bool need_fail = false;

    std::optional<int> autoritize(const std::string& token) override {
      ++call_autoritize;
      last_token = token;
      if(need_fail) return std::nullopt;
      return mock_user_id;
    }
};

class AutoritizerProvider {
  public:
    static void set(std::shared_ptr<IAutoritizer> a) {
      instance() = std::move(a);
    }

    static std::shared_ptr<IAutoritizer> get() {
      return instance();
    }

  private:
    static std::shared_ptr<IAutoritizer>& instance() {
      static std::shared_ptr<IAutoritizer> inst =
          std::make_shared<RealAutoritizer>();
      return inst;
    }
};

#endif // AUTORITIZERPROVIDER_H
