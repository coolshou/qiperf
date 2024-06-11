#include "QIPConfig.h"

#include <QDebug>

const QByteArray QIPConfig::MAGIC_VALUE = ".QIP";
const QByteArray QIPConfig::VERSION = "1";

QIPConfig::QIPConfig(QObject *parent):
    QObject(parent)
{
    //init value
    m_data= new QIPConfigData();
    m_version = 1;
    m_data->tpcfg = "";

}

bool QIPConfig::loadFromFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    QDataStream in(file.readAll());
    file.close();
    // QByteArray fdata = file.readAll();
    in >> m_magic;
    in >> m_version;
    if (m_magic.startsWith(MAGIC_VALUE)){
        QByteArray compressedData;
        in >> compressedData;

        QByteArray data = qUncompress(compressedData);
        if (data.isEmpty()) {
            qDebug() << "ERROR: Wrong format of the config file: " << filePath;
            return false;
        }
        return deserialize(data);
    } else {
        return false;
    }
}

bool QIPConfig::saveToFile(const QString &filePath) const {
    QByteArray data = serialize();
    QByteArray compressedData = qCompress(data, 9);

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(MAGIC_VALUE+VERSION+compressedData);
    file.flush();
    file.close();

    return true;
}

uint32_t QIPConfig::getVersion()
{
    return m_version;
}

QByteArray QIPConfig::getTPCfg()
{
    return m_data->tpcfg.toUtf8();
}

void QIPConfig::setTPCfg(QByteArray tpcfg)
{
    m_data->tpcfg.fromUtf8(tpcfg);
}

QByteArray QIPConfig::serialize() const {
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15); // Set the stream version

    out << m_data->tpcfg;

    return data;
}

bool QIPConfig::deserialize(const QByteArray &data) {
    QDataStream in(data);
    in.setVersion(QDataStream::Qt_5_15); // Set the stream version

    in >> m_data->tpcfg;

    return !in.status();
}
