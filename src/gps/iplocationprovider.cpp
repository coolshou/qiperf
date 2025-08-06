#include "iplocationprovider.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QNetworkRequest>

IpLocationProvider::IpLocationProvider(QObject* parent)
    : QObject(parent), manager(new QNetworkAccessManager(this)) {
    connect(manager, &QNetworkAccessManager::finished,
            this, &IpLocationProvider::handleReply);
}

void IpLocationProvider::fetchLocation() {
    QUrl url("http://ip-api.com/json");
    manager->get(QNetworkRequest(url));
}

void IpLocationProvider::handleReply(QNetworkReply* reply) {
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll());
        QJsonObject obj = jsonDoc.object();

        IpLocation loc;
        loc.city = obj["city"].toString();
        loc.region = obj["regionName"].toString();
        loc.country = obj["country"].toString();
        loc.latitude = obj["lat"].toDouble();
        loc.longitude = obj["lon"].toDouble();

        emit locationReady(loc);
    } else {
        emit locationError(reply->errorString());
    }
    reply->deleteLater();
}
