#ifndef IPLOCATIONPROVIDER_H
#define IPLOCATIONPROVIDER_H

#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

struct IpLocation {
    QString city;
    QString region;
    QString country;
    double latitude;
    double longitude;
};

class IpLocationProvider : public QObject {
    Q_OBJECT
public:
    explicit IpLocationProvider(QObject* parent = nullptr);
    void fetchLocation();

signals:
    void locationReady(const IpLocation& location);
    void locationError(const QString& error);

private slots:
    void handleReply(QNetworkReply* reply);

private:
    QNetworkAccessManager* manager;
};

#endif // end IPLOCATIONPROVIDER_H
