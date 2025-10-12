#ifndef MOCKREPLY_H
#define MOCKREPLY_H
#include <QNetworkReply>

class MockReply : public QNetworkReply
{
    Q_OBJECT

public:

    MockReply(QObject* parent = nullptr) : QNetworkReply(parent) {
        open(ReadOnly | Unbuffered);
        setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        setFinished(true);
        setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
        setUrl(QUrl("http://mock.url"));
    }

    void abort() override {  }
    void setData(const QByteArray& d) { data = d; }
    void emitFinished() { Q_EMIT finished(); }

    QByteArray data;

protected:

    qint64 readData(char* buffer, qint64 maxlen) override {
        qint64 len = std::min(maxlen, qint64(data.size()));
        memcpy(buffer, data.constData(), len);
        data.remove(0, len);
        return len;
    }
};

#endif // MOCKREPLY_H
