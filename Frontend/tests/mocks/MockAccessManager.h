#ifndef MOCKACCESSMANAGER_H
#define MOCKACCESSMANAGER_H

#include <QTimer>

#include "Debug_profiling.h"
#include "MockReply.h"
#include "interfaces/INetworkAccessManager.h"

class MockNetworkAccessManager : public INetworkAccessManager {
public:
  MockNetworkAccessManager(MockReply *mock_reply) : reply(mock_reply) {}

  bool shouldFail = false;
  bool shouldReturnResponce = true;

  MockReply *post(const QNetworkRequest &req,
                  const QByteArray &byte_array) override {
    last_data = byte_array;
    last_request = req;
    ++post_counter;
    if (!reply)
      LOG_WARN("[MockNetworkAccessManager] Reply in nullptr");

    if (!shouldReturnResponce)
      return reply;

    if (shouldFail)
      QTimer::singleShot(0, reply, &MockReply::errorOccurred);
    else
      QTimer::singleShot(0, reply, &MockReply::emitFinished);
    return reply;
  }

  MockReply *get(const QNetworkRequest &req) override {
    last_request = req;
    ++get_counter;
    if (!reply)
      LOG_WARN("[MockNetworkAccessManager] Reply in nullptr");

    if (!shouldReturnResponce)
      return reply;

    if (shouldFail)
      QTimer::singleShot(0, reply, &MockReply::errorOccurred);
    else
      QTimer::singleShot(0, reply, &MockReply::emitFinished);
    return reply;
  }

  auto put(const QNetworkRequest &, const QByteArray &byte_array)
      -> QNetworkReply * override {}
  auto del(const QNetworkRequest &) -> QNetworkReply * override {}

  QByteArray last_data;
  QNetworkRequest last_request;
  int post_counter = 0;
  int get_counter = 0;

  void setReply(MockReply *reply) { this->reply = reply; }

private:
  MockReply *reply = nullptr;
};

#endif // MOCKACCESSMANAGER_H
