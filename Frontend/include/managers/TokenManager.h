#ifndef TOKENMANAGER_H
#define TOKENMANAGER_H

#include <optional>
#include <QString>

#include "Debug_profiling.h"

class TokenManager {
    std::optional<QString> token_;
    std::optional<long long> current_id_;
  public:
    // TokenManager() = default;
    // TokenManager(const QString& token, long long current_id) : token_(token), current_id_(current_id) {}

    void setData(const QString& token, long long current_id) {
      token_ = token;
      current_id_ = current_id;
    }

    QString getToken() {
      if(!token_.has_value()) {
        LOG_ERROR("Token not setted");
        throw std::runtime_error("Token not setted");
      }

      return *token_;
    }

    long long getCurrentUserId() {
      if(!current_id_.has_value()) {
        LOG_ERROR("Id not setted");
        throw std::runtime_error("Id not setted");
      }

      return *current_id_;
    }

    void resetData() {
      token_.reset();
      current_id_.reset();
    }
};

#endif // TOKENMANAGER_H
