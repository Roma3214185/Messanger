#ifndef MOCKACCESSMANAGER_H
#define MOCKACCESSMANAGER_H

#include <QTimer>

#include "Debug_profiling.h"
#include "MockReply.h"
#include "interfaces/INetworkAccessManager.h"

class MockNetworkAccessManager : public INetworkAccessManager {
 public:
  MockNetworkAccessManager(QNetworkReply* mock_reply) : reply(mock_reply) {}
  QNetworkReply* post(const QNetworkRequest& req, const QByteArray& byte_array) override {
    last_data    = byte_array;
    last_request = req;
    ++post_counter;
    return reply;
  }

  QNetworkReply* get(const QNetworkRequest& req) override {
    last_request = req;
    ++get_counter;
    if (!reply) LOG_WARN("[MockNetworkAccessManager] Reply in nullptr");
    return reply;
  }

  QByteArray      last_data;
  QNetworkRequest last_request;
  int             post_counter = 0;
  int             get_counter  = 0;

  void setReply(QNetworkReply* reply) { this->reply = reply; }

 private:
  QNetworkReply* reply = nullptr;
};

#endif  // MOCKACCESSMANAGER_H
