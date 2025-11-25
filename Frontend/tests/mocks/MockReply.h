#ifndef TESTS_MOCKS_MOCKREPLY_H
#define TESTS_MOCKS_MOCKREPLY_H

#include <QByteArray>
#include <QNetworkReply>

class MockReply : public QNetworkReply {
 public:
  MockReply(QObject* parent = nullptr) : QNetworkReply(parent) {
    open(ReadOnly | Unbuffered);
    setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    setFinished(true);
    setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
    setUrl(QUrl("http://mock.url"));
  }

  QString mock_str = "Mock network error";
  QNetworkReply::NetworkError mock_code = ConnectionRefusedError;

  void setMockError(QNetworkReply::NetworkError code, const QString& str) {
    setError(code, str);
    mock_str = str;
    mock_code = code;
  }

  void abort() override {}
  void setData(const QByteArray& data) { this->data = data; }

  QByteArray data;

 public Q_SLOTS:
  void emitFinished() {
    setFinished(true);
    setError(NoError, {});
    Q_EMIT finished();
  }

  void errorOccurred() {
    setFinished(true);
    setError(mock_code, mock_str);
    Q_EMIT QNetworkReply::errorOccurred(mock_code);
    Q_EMIT finished();
  }

 protected:
  qint64 readData(char* buffer, qint64 maxlen) override {
    qint64 len = std::min(maxlen, qint64(data.size()));
    memcpy(buffer, data.constData(), len);
    data.remove(0, len);
    return len;
  }
};

#endif  // TESTS_MOCKS_MOCKREPLY_H
