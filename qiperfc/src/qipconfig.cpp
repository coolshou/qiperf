#include "qipconfig.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDir>
#include <QPixmap>
#include <QByteArray>
#include <QIODevice>
#include <QBuffer>
#include <QNetworkProxyQuery>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QUrl>

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
    qDeleteAll(m_fileworkers);
    m_fileworkers.clear();

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Can not read file: " << QDir::toNativeSeparators(filePath);
        return false;
    }
    bool rc=false;
    QDataStream in_lff(&file);
    in_lff >> m_magic;
    in_lff >> m_loadversion;
    if (m_magic.startsWith(MAGIC_VALUE)){
        QByteArray compressedtpcfg;
        in_lff >> compressedtpcfg;

        QByteArray data = qUncompress(compressedtpcfg);
        if (data.isEmpty()) {
            qDebug() << "ERROR: Wrong format of the config file: " << filePath;
            return false;
        }
        if (deserialize(data)){
            if (m_loadversion>=2){
                QByteArray compressedfiles;
                //tmp path
                if (m_data->testdate != "") {
                    QString outpath = m_tmppath + QDir::separator() + m_data->testdate;
                    emit updateDataPath(outpath);
                    in_lff >> compressedfiles;
                    rc = filesFromStore(compressedfiles, outpath);
                    if (rc){
                        rc = parserTPCfgLogFiles(outpath);
                    }
                }else{
                    qDebug() << "no test record date";
                    rc = true;
                }
            }else {
                qDebug() << "config file version is old: " << m_loadversion;
                rc = true;
            }
        }else {
            qDebug() << "ERROR: Wrong format of the data: " << filePath;
            rc = false;
        }
        file.close();
        return rc;
    } else {
        qDebug() << "Wrong format of " << filePath;
        file.close();
        return false;
    }
}

bool QIPConfig::saveToFile(const QString &filePath) const {
    QByteArray data = serialize();
    QByteArray compressedtpcfg = qCompress(data, 9);
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Can not write file: " << QDir::toNativeSeparators(filePath);
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

void QIPConfig::clear()
{ //clear test data
    m_data->testdate = "";

}

bool QIPConfig::importIperf3Log(QString filename)
{
    QFile inputFile(filename);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Can not read file: " << QDir::toNativeSeparators(filename);
        return false;
    }
    QTextStream in(&inputFile);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        //TODO: header parser, to get following info
        // QString version, QString protocal,
        // int idx, bool servermode, int parallel,
        // bool bidir, QString bidirtag , QString filename,

        qDebug() << "line: " << line;
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    inputFile.close();
    //TODO: use IperfFileWorker to do parser
    return true;
}

bool QIPConfig::exportToFile(QString filename, TPPlot *tpplot, int tpwidth, int tpheigth)
{
    //TODO: how to constructure html file!!

    //TODO: following is just a test to save plot to png file!!
    QString img = filename.replace(".html", ".png");
    qDebug() << "save throughput plot to png: " << img;
    QPixmap tp = tpplot->toPixmap(tpwidth, tpheigth); //TODO: into html file
    if (!tp.save(img)){
        // if (!tpplot->savePng(img)){
        qDebug() << "throughput plot save to " << img << " Fail!";
    }
}

bool QIPConfig::detectSystemProxy(QString &hostname, quint16 &port)
{
    QNetworkProxyQuery npq(QUrl("http://www.google.com"));
    QList<QNetworkProxy> listOfProxies = QNetworkProxyFactory::systemProxyForQuery(npq);
    if(!listOfProxies.isEmpty()) {
        QNetworkProxy proxy = listOfProxies.first();  // Take the first proxy (if multiple proxies are available)
        if (proxy.type() != QNetworkProxy::NoProxy) {
            qDebug() << "Proxy found!";
            hostname = proxy.hostName();
            qDebug() << "Proxy Host:" << hostname;
            port = proxy.port();
            qDebug() << "Proxy Port:" << port;
            qDebug() << "Proxy Type:" << proxy.type();
            return true;
        }else{
            return false;
        }
    }else{
        return false;
    }
}

void QIPConfig::onProgress(int currentlineno)
{
    emit progress(currentlineno);
}

void QIPConfig::onUpdateTPAvg(QString midx, QString sInterval, QString idx,
                              QString value, QString unit, QString dir,
                              QString pkt_lost, QString pkt_total)
{
    emit updateTPAvg(midx, sInterval, idx,
                     value, unit, dir,
                     pkt_lost, pkt_total);
}

void QIPConfig::onThroughputData(int idx, QString sInterval, QString data)
{
//    qDebug() << "QIPConfig::onThroughputData: " << idx << " sInterval: " << sInterval << " data: " << data;
    emit onThroughput(QString::number(idx), sInterval, data);
}

void QIPConfig::onUpdateTPDatas(QString refrow, QVector<double> timedatas, QVector<double> valuedatas,
                                QVector<int> packetlosts, QVector<int> packettotals, QVector<double> lostrate)
{
    emit updateTPDatas(refrow, timedatas, valuedatas, packetlosts, packettotals, lostrate);
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
    QDataStream in_de(data);
    in_de.setVersion(QDataStream::Qt_5_15); // Set the stream version

    in_de >> m_data->tpcfg;
    emit updateTPCfg(m_data->tpcfg.toUtf8());
    if (m_loadversion>=2){
        in_de >> m_data->env;
        in_de >> m_data->testdate;
//        qDebug() << "m_data env:" << m_data->env;
        // qDebug() << "m_data testdate:" << m_data->testdate;
        emit updateStartDateTime(QDateTime::fromString(m_data->testdate, DATETIME_NOW_FORMAT));
    }
    return !in_de.status();
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
//        qDebug() << "filesToStore name:" << name;
        out << name << compressedData;

        inputFile.close();
        QCoreApplication::processEvents(QEventLoop::AllEvents);
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
        QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    return true;
}

bool QIPConfig::parserTPCfgLogFiles(QString logpath)
{
//    qDebug() << "parserTPCfgLogFiles m_fileworkers:" << m_fileworkers.length();
    //use m_data->tpcfg to parser all log file to setup throughput plot chart
    QDir d;
    if (m_data->tpcfg.size()>0){
//        qDebug() << "parserTPCfgLogFiles tpcfg: " << m_data->tpcfg;
        QJsonParseError error;
        QJsonDocument doc=QJsonDocument::fromJson(m_data->tpcfg.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError){
            QJsonObject jClient;
            QJsonObject jServer;
            QJsonArray arr = doc.array();
//            qDebug() << "QIPConfig::parserTPCfgLogFiles: " << arr;
            int idx=0;
//            foreach(auto jObj, arr){
            for(QJsonArray::const_iterator it=arr.constBegin(); it!=arr.constEnd(); ++it){
                //TODO: other type of "Action"
                QJsonObject jObj = it->toObject();
//                qDebug() << "QIPConfig QJsonObject: " << jObj;
                if (jObj.value("Action").toString() == "IPERF_ADD" &&
                        jObj.value("enabled").toBool(true)){
                        //client
                    jClient = jObj.value("client").toObject();
                    QString clientip = jClient.value("bind").toString();
                    bool bidir = jClient.value("bidir").toBool();
                    bool reverse = jClient.value("reverse").toBool();
                    QString protocal = jClient.value("protocal").toString();
                    int parallel = jClient.value("parallel").toInt();
                    QString version = jClient.value("version").toString();
                    int clientport = jClient.value("port").toInt();
                    //server
                    jServer = jObj.value("server").toObject();
                    QString serverip = jServer.value("bind").toString();
                    int serverport = jServer.value("port").toInt();
                    QString serverfile = logpath + QDir::separator() + serverip + "_" +QString::number(serverport)+ ".log";
                    QString clientfile = logpath + QDir::separator() + clientip + "-" + serverip + "_" +QString::number(clientport)+ ".log";
                    if (!bidir){
                        if (!reverse){
                            if (d.exists(serverfile)){
                                //iperf server record file
                                IperfFileWorker *ifw = new IperfFileWorker(version, protocal,
                                                                           idx, true, parallel,
                                                                           bidir, "Tx", serverfile);
                                m_fileworkers.append(ifw);
                                connect(ifw, &IperfFileWorker::onThroughput, this, &QIPConfig::onThroughputData);
                                connect(ifw, &IperfFileWorker::updateTPDatas, this, &QIPConfig::onUpdateTPDatas);
                                connect(ifw, &IperfFileWorker::updateTPAvg, this, &QIPConfig::onUpdateTPAvg);
                                connect(ifw, &IperfFileWorker::progress, this, &QIPConfig::onProgress);
                                ifw->start();
                            }else{
                                qDebug() << "not exist serverfile:" << serverfile;
                            }
                        }else{
                            if (d.exists(clientfile)){
                                //iperf client record file
                                IperfFileWorker *ifwc = new IperfFileWorker(version, protocal,
                                                                           idx, false, parallel,
                                                                           bidir, "Rx", clientfile);
                                m_fileworkers.append(ifwc);
                                connect(ifwc, &IperfFileWorker::onThroughput, this, &QIPConfig::onThroughputData);
                                connect(ifwc, &IperfFileWorker::updateTPDatas, this, &QIPConfig::onUpdateTPDatas);
                                connect(ifwc, &IperfFileWorker::updateTPAvg, this, &QIPConfig::onUpdateTPAvg);
                                connect(ifwc, &IperfFileWorker::progress, this, &QIPConfig::onProgress);
                                ifwc->start();
                            }else{
                                qDebug() << "not exist clientfile:" << clientfile;
                            }
                        }
                    }else{
                        if (d.exists(serverfile)){
                            IperfFileWorker *ifw = new IperfFileWorker(version, protocal,
                                                                       idx, true, parallel,
                                                                       bidir, "Tx", serverfile);
                            m_fileworkers.append(ifw);
                            connect(ifw, &IperfFileWorker::onThroughput, this, &QIPConfig::onThroughputData);
                            connect(ifw, &IperfFileWorker::updateTPDatas, this, &QIPConfig::onUpdateTPDatas);
                            connect(ifw, &IperfFileWorker::updateTPAvg, this, &QIPConfig::onUpdateTPAvg);
                            connect(ifw, &IperfFileWorker::progress, this, &QIPConfig::onProgress);
                            ifw->start();
                        }else{
                            qDebug() << "bidir: not exist serverfile:" << serverfile;
                        }
                        if (d.exists(clientfile)){
                            IperfFileWorker *ifwc = new IperfFileWorker(version, protocal,
                                                                       idx, false, parallel,
                                                                       bidir, "Rx", clientfile);
                            m_fileworkers.append(ifwc);
                            connect(ifwc, &IperfFileWorker::onThroughput, this, &QIPConfig::onThroughputData);
                            connect(ifwc, &IperfFileWorker::updateTPDatas, this, &QIPConfig::onUpdateTPDatas);
                            connect(ifwc, &IperfFileWorker::updateTPAvg, this, &QIPConfig::onUpdateTPAvg);
                            connect(ifwc, &IperfFileWorker::progress, this, &QIPConfig::onProgress);
                            ifwc->start();
                        }else{
                            qDebug() << "bidir: not exist clientfile:" << clientfile;
                        }
                    }
                }
                idx = idx +1;
                QCoreApplication::processEvents(QEventLoop::AllEvents);
            }
            return true;
        }else{
            qDebug() << "Wrong format of m_data->tpcfg: " << error.errorString();
            return false;
        }
    }
    qDebug() << "No data of m_data->tpcfg";
    return false;
}
