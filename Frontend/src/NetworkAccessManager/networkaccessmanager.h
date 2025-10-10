#ifndef NETWORKACCESSMANAGER_H
#define NETWORKACCESSMANAGER_H
#include <QNetworkAccessManager>
#include "headers/INetworkAccessManager.h"

class NetworkAccessManager : public INetworkAccessManager
{
    QNetworkAccessManager netManager;
public:
    QNetworkReply* post(const QNetworkRequest& req, const QByteArray& byteArray) override { return netManager.post(req, byteArray); }
    QNetworkReply* get(const QNetworkRequest& req) override { return netManager.get(req); }
    //QNetworkReply* post(const QNetworkRequest& req, const QByteArray& byteArray) override { return netManager.post(req, byteArray); }
};

#endif // NETWORKACCESSMANAGER_H
