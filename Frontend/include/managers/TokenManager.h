#ifndef TOKENMANAGER_H
#define TOKENMANAGER_H

#include <optional>
#include <QString>

#include "Debug_profiling.h"

class TokenManager {
    std::optional<QString> token_;
  public:
    TokenManager() = default;
    TokenManager(const QString& token) : token_(token) {}

    void setToken(const QString& token) {
      token_ = token;
    }

    QString getToken() {
      if(!token_.has_value()) {
        LOG_ERROR("Token not setted");
        throw std::runtime_error("Token not setted");
      }

      return *token_;
    }

    void resetToken() {
      token_.reset();
    }
};

#endif // TOKENMANAGER_H
