#ifndef MOCKACCESSMANAGER_H
#define MOCKACCESSMANAGER_H

#include "headers/INetworkAccessManager.h"
#include "headers/MockReply.h"

class MockNetworkAccessManager : public INetworkAccessManager{

public:

    QNetworkReply* post(const QNetworkRequest& req, const QByteArray& byteArray) override{
        Q_UNUSED(byteArray)
        lastRequest = req;
        return reply;
    }

    QNetworkReply* get(const QNetworkRequest& req) override{
        lastRequest = req;
        return reply;
    }

    auto getLastRequest() const -> QNetworkRequest{
        return lastRequest;
    }

    void setReply(QNetworkReply* reply){
        this->reply = reply;
    }

private:

    QNetworkRequest lastRequest;
    QNetworkReply* reply = nullptr;
};

#endif // MOCKACCESSMANAGER_H
