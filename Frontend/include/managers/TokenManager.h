#ifndef TOKENMANAGER_H
#define TOKENMANAGER_H

#include <QString>
#include <optional>

#include "Debug_profiling.h"

class TokenManager {
  std::optional<QString> token_;
  std::optional<long long> current_id_;

 public:
  void setData(const QString &token, long long current_id) {
    DBC_REQUIRE(!token.isEmpty());
    DBC_REQUIRE(current_id > 0);
    token_ = token;
    current_id_ = current_id;
  }

  QString getToken() {
    DBC_REQUIRE(token_ != std::nullopt);
    if (!token_) {
      throw std::runtime_error("token is empty");
    }
    return token_.value();
  }

  long long getCurrentUserId() {
    DBC_REQUIRE(current_id_ != std::nullopt);
    if (!current_id_.has_value()) {
      throw std::runtime_error("current_id_ is empty");
    }
    return current_id_.value();
  }

  void resetData() {
    token_.reset();
    current_id_.reset();
  }
};

#endif  // TOKENMANAGER_H
