#ifndef INETWORKACCESSMANAGER_H
#define INETWORKACCESSMANAGER_H

#include <QNetworkReply>

class INetworkAccessManager{
public:
    virtual QNetworkReply* post(const QNetworkRequest&, const QByteArray&) = 0;
    virtual QNetworkReply* get(const QNetworkRequest&) = 0;
    //virtual QNetworkReply* post(const QNetworkRequest& req, const QByteArray& byteArray) = 0;
};

#endif // INETWORKACCESSMANAGER_H
