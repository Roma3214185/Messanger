#ifndef MOCKACCESSMANAGER_H
#define MOCKACCESSMANAGER_H

#include "headers/INetworkAccessManager.h"
#include "headers/MockReply.h"

class MockNetworkAccessManager : public INetworkAccessManager {
 public:
  QNetworkReply* post(const QNetworkRequest& req,
                      const QByteArray& byte_array) override {
    Q_UNUSED(byte_array)
    last_request_ = req;
    return reply;
  }

  QNetworkReply* get(const QNetworkRequest& req) override {
    last_request_ = req;
    return reply;
  }

  auto getLastRequest() const -> QNetworkRequest { return last_request_; }

  void setReply(QNetworkReply* reply) { this->reply = reply; }

 private:
  QNetworkRequest last_request_;
  QNetworkReply* reply = nullptr;
};

#endif  // MOCKACCESSMANAGER_H
