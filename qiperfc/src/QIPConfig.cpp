#include "QIPConfig.h"

#include <QDebug>

const QByteArray QIPConfig::MAGIC_VALUE = ".QIP";
const qint32 QIPConfig::VERSION = 2;

QIPConfig::QIPConfig(QString tmppath, QObject *parent):
    QObject(parent), m_tmppath(tmppath)
{
    //init value
    m_data= new QIPConfigData();
    m_version = 2;
    m_data->tpcfg = "";
    m_data->env = "";
    m_data->testdate = "";


}

bool QIPConfig::loadFromFile(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
//    QDataStream in(file.readAll());
    QDataStream in(&file);
//    file.close();
    // QByteArray fdata = file.readAll();
    in >> m_magic;
    in >> m_loadversion;
    if (m_magic.startsWith(MAGIC_VALUE)){
        QByteArray compressedtpcfg;
        in >> compressedtpcfg;

        QByteArray data = qUncompress(compressedtpcfg);
        if (data.isEmpty()) {
            qDebug() << "ERROR: Wrong format of the config file: " << filePath;
            return false;
        }
        file.close();
        if (deserialize(data)){
            if (m_loadversion>=2){
                QByteArray compressedfiles;
                //tmp path
                QString outpath = m_tmppath + QDir::separator() + m_data->testdate;
                in >> compressedfiles;
                return filesFromStore(compressedfiles, outpath);
            }else {
                return true;
            }
        }else {
            qDebug() << "ERROR: Wrong format of the data: " << filePath;
            return false;
        }

    } else {
        file.close();
        qDebug() << "Wrong format of " << filePath;
        return false;
    }
}

bool QIPConfig::saveToFile(const QString &filePath) const {
    QByteArray data = serialize();
    QByteArray compressedtpcfg = qCompress(data, 9);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    QDataStream out(&file);
    out << static_cast<QByteArray>(MAGIC_VALUE);
    out << static_cast<qint32>(VERSION);
    out << static_cast<QByteArray>(compressedtpcfg);
    // compressed data records
    if (m_version>=2){
        QByteArray compressedfiles = filesToStore(m_data->datafilenames);
        out << compressedfiles;
    }

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

void QIPConfig::setTPCfg(QByteArray tpcfg, QString env, QString testdate, QStringList datafilenames)
{
    m_data->tpcfg= QString::fromUtf8(tpcfg);
    m_data->env = env;
    if (!testdate.isEmpty()){
        m_data->testdate = testdate;
        m_data->datafilenames = datafilenames;
    }
}

QByteArray QIPConfig::serialize() const {
    QByteArray data;
    QDataStream out(&data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15); // Set the stream version

    out << m_data->tpcfg;
    if (m_version>=2){
        out << m_data->env;
        out << m_data->testdate;
    }

    return data;
}

bool QIPConfig::deserialize(const QByteArray &data) {
    QDataStream in(data);
    in.setVersion(QDataStream::Qt_5_15); // Set the stream version

    in >> m_data->tpcfg;
    if (m_loadversion>=2){
        in >> m_data->env;
        in >> m_data->testdate;
    }

    return !in.status();
}

QByteArray QIPConfig::filesToStore(QStringList &inputFiles) const
{
    QByteArray output;
    QDataStream out(&output, QIODevice::WriteOnly);

    for (const QString &fileName : inputFiles) {
        QFile inputFile(fileName);
        if (!inputFile.open(QIODevice::ReadOnly)) {
            qWarning() << "Cannot open input file" << fileName << "for reading:" << inputFile.errorString();
            continue;
        }

        QByteArray fileContent = inputFile.readAll();
        QFileInfo fileInfo(inputFile);
        QString name = fileInfo.fileName();
        //out << name << fileContent;

        // Compress the file content
        QByteArray compressedData = qCompress(fileContent, 9);
        qDebug() << "name:" <<name;
        out << name << compressedData;

        inputFile.close();
    }

    //outputFile.close();
    return output;
}

bool QIPConfig::filesFromStore(QByteArray &inputData, const QString &outputFolder) const
{
    QDir dir(outputFolder);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qCritical() << "Cannot create output directory:" << outputFolder;
            return false;
        }
    }

    // QFile inputFile(filename);
    // if (!inputFile.open(QIODevice::ReadOnly)) {
    //     qCritical() << "Cannot open input file for reading:" << inputFile.errorString();
    //     return false;
    // }
    QDataStream in(inputData);
    while (!in.atEnd()) {
        QString name;
        QByteArray compressedData;

        in >> name >> compressedData;

        // Decompress the file content
        QByteArray fileContent = qUncompress(compressedData);

        // Optionally write the decompressed data to a file or process it as needed
        QString outputFilePath = dir.filePath(name);
        QFile outputFile(outputFilePath);
        if (!outputFile.open(QIODevice::WriteOnly)) {
            qWarning() << "Cannot open output file" << name << "for writing:" << outputFile.errorString();
            continue;
        }

        outputFile.write(fileContent);
        outputFile.close();
    }

    // inputFile.close();
    return true;
}
