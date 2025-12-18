#include "aip.h"

AIP::AIP(QObject *parent)
    : QObject{parent}
{}

AIP::~AIP() {}

void AIP::initBeamData(QString filename)
{
    Q_UNUSED(filename)
}

QStringList AIP::sorted(QStringList datas)
{
    QStringList list = datas;
    std::sort(list.begin(), list.end(), [](const QString &a, const QString &b) {
        return a.toInt() < b.toInt();
    });
    return list;
}
