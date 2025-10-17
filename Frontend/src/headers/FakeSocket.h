#ifndef FAKESOCKET_H
#define FAKESOCKET_H

#include <QObject>
#include <QWebSocket>

class FakeSocket : public QWebSocket{

public:

    void open(const QUrl& url) {
        Q_UNUSED(url)
        Q_EMIT connected();
    }

    void sendTextMessage(const QString& msg) {
        ++sendTextMessage_calls;
    }

    int sendTextMessage_calls = 0;
};

#endif // FAKESOCKET_H
