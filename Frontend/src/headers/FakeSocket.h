#ifndef FAKESOCKET_H
#define FAKESOCKET_H

#include <QObject>
#include <QWebSocket>

class FakeSocket : public QWebSocket {
public:
    int sendTextMessage_calls = 0;

    void open(const QUrl& url) {
        Q_UNUSED(url)
        Q_EMIT connected();
    }

    void sendTextMessage(const QString& msg) {
        qDebug() << "[FakeSocket] send: " << msg;
        ++sendTextMessage_calls;
    }
};

#endif // FAKESOCKET_H
