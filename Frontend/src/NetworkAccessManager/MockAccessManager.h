#ifndef MOCKACCESSMANAGER_H
#define MOCKACCESSMANAGER_H

#include "headers/INetworkAccessManager.h"
#include "headers/MockReply.h"

class MockNetworkAccessManager : public INetworkAccessManager
{
    QNetworkRequest lastRequest;
    QNetworkReply* reply = nullptr;
public:
    QNetworkReply* post(const QNetworkRequest& req, const QByteArray& byteArray) override {
        qDebug() << "post";
        Q_UNUSED(byteArray)
        lastRequest = req;
        return reply;
    }

    QNetworkReply* get(const QNetworkRequest& req) override{
        qDebug() << "get";
        lastRequest = req;
        return reply;
    }

    QNetworkRequest getLastRequest() const{
        return lastRequest;
    }

    void setReply(QNetworkReply* reply){
        this->reply = reply;
    }

};

#endif // MOCKACCESSMANAGER_H
