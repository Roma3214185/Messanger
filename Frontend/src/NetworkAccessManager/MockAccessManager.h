#ifndef MOCKACCESSMANAGER_H
#define MOCKACCESSMANAGER_H

#include "headers/INetworkAccessManager.h"
#include "headers/MockReply.h"
#include <QTimer>

class MockNetworkAccessManager : public INetworkAccessManager {
 public:
  QNetworkReply* post(const QNetworkRequest& req,
                      const QByteArray& byte_array) override {
    last_data = byte_array;
    last_request = req;
    ++post_counter;
    return reply;
  }

  QNetworkReply* get(const QNetworkRequest& req) override {
    last_request = req;
    ++get_counter;
    return reply;
  }

  QByteArray last_data;
  QNetworkRequest last_request;
  int post_counter = 0;
  int get_counter = 0;

  void setReply(QNetworkReply* reply) { this->reply = reply; }

 private:
  QNetworkReply* reply = nullptr;
};

#endif  // MOCKACCESSMANAGER_H
