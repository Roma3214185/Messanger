#ifndef TOKENMANAGER_H
#define TOKENMANAGER_H

#include <QString>
#include <optional>

class TokenManager {
 public:
  void setData(const QString &token, long long current_id);
  QString getToken() noexcept(false);
  long long getCurrentUserId() noexcept(false);
  void resetData();

 private:
  std::optional<QString> token_;
  std::optional<long long> current_user_id_;
};

#endif  // TOKENMANAGER_H
