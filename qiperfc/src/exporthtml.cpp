#include "exporthtml.h"

#include <QWebChannel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFile>
#include <QNetworkProxyFactory>
#include <QByteArray>
#include <QBuffer>
#include <QIODevice>
#include <QList>
#include <QPixmap>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QWebEngineSettings>

ExportHtml::ExportHtml(QString templatefile, QString savefile,int width, int heigth,
                       QWidget *parent)
    : QWidget{parent}, m_templatefile(templatefile), m_savefile(savefile),
    m_width(width), m_heigth(heigth)
{
    m_iperf_raw_filenames.clear();
    m_iperfwrapper = new IperfWrapper();
    // m_ok = false;
    connect(this, &ExportHtml::ready, this, &ExportHtml::procressData);
    QNetworkProxyFactory::setUseSystemConfiguration(false); // not use system proxy
    QVBoxLayout *layout = new QVBoxLayout(this);
    webView = new QWebEngineView(this);
    connect(webView, &QWebEngineView::loadFinished, this, &ExportHtml::onLoadFinished);
    webView->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    // QPushButton *addButton = new QPushButton("add Tag", this);
    layout->addWidget(webView);
    // layout->addWidget(addButton);
    // connect(addButton, &QPushButton::clicked, this, &ExportHtml::onAddTag);
    setLayout(layout);
    // loadhtml(m_templatefile);// load template file

    // Enable developer tools
    // Initialize developer tools window
    // devTools = new QWebEngineView(this);
    // layout->addWidget(devTools);
    // devTools->setWindowTitle("Developer Tools");
    // devTools->resize(1280,1024);
}

ExportHtml::~ExportHtml()
{
    webView->deleteLater();
}

void ExportHtml::loadhtml(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot open input file" << filename << "for reading:" << file.errorString();
        return;
    }
    QString htmlContent = file.readAll();
    webView->setHtml(htmlContent);

    //procress all data?
}

void ExportHtml::AddDivRow(QString pId, QList<QString> values)
{
    QString js = "";
    js.append(QString("var pTag = document.getElementById('%1');").arg(pId));
    js.append(QString("var trTag = document.createElement('div');trTag.classList.add('table-tr');"));
    int idx=0;
    int count = values.count();
    QString td="td";
    foreach (QString value, values) {
        js.append(QString("var tdTag%1 = document.createElement('div');").arg(idx));
        if (idx == (count-1)) {
            td="tdr";
        }
        js.append(QString("tdTag%1.classList.add('table-%2');").arg(QString::number(idx), td));
        js.append(QString("tdTag%1.innerHTML = '%2';").arg(QString::number(idx), value));
        js.append(QString("trTag.appendChild(tdTag%1);").arg(idx));
        idx++;
    }
    js.append(QString("pTag.appendChild(trTag);"));
    if (pId.contains("RawData")){
        qDebug() << "AddDivRow RawData JS: " << js;
    }
    webView->page()->runJavaScript(js);
}

void ExportHtml::AddRawData(QString pId, QString name, QString data)
{
    QString js = "";
    QString idname = "Raw_"+ QString::number(QRandomGenerator::global()->bounded(65535));

    js.append(QString("var pTag = document.getElementById('%1');").arg(pId));
    js.append(QString("var trTag = document.createElement('div');"));
    js.append(QString("trTag.classList.add('table-tr');"));
    //name
    js.append(QString("var tdTag_1 = document.createElement('div');"));
    js.append(QString("tdTag_1.classList.add('table-tdraw');"));
    js.append(QString("tdTag_1.innerHTML = '%1';").arg(name));
    js.append(QString("trTag.appendChild(tdTag_1);"));
    //data
    js.append(QString("var tdTag_2 = document.createElement('div');"));
    js.append(QString("tdTag_2.classList.add('table-tdraw');"));
    js.append(QString("var tdTag_data = document.createElement('pre');"));
    js.append(QString("tdTag_data.setAttribute('id', '%1');").arg(idname));

    js.append(QString("tdTag_2.appendChild(tdTag_data);"));
    js.append(QString("trTag.appendChild(tdTag_2);"));

    js.append(QString("pTag.appendChild(trTag);"));
#if (DEBUG_EXPORT_HTML==1)
    if (pId.contains("RawData")){
        qDebug() << "AddRawData RawData JS: " << js;
    }
#endif
    webView->page()->runJavaScript(js);

    //data
    // Encode content in Base64
    QByteArray base64Content = data.toUtf8().toBase64();

    js = QString("var decodedContent = atob('%1');").arg(QString::fromLatin1(base64Content));
    js.append(QString("document.getElementById('%1').innerHTML = decodedContent;").arg(idname));
    webView->page()->runJavaScript(js);

}

void ExportHtml::AddDivPng(QString pId, QString sImg)
{   //pID: css tag name
    //sImg: base64 encode image string
    QString js = "";
    // add <style> tag to <head>
    js.append(QString("const style = document.createElement('style');"));
    js.append(QString("style.textContent = '.%1  { background-image: url(\"data:image/png;base64,%2\");\
background-repeat: no-repeat;\
background-attachment: inherit;\
background-size: contain;\
background-position: center;\
width: 100%;\
height: 100%;\
display: block;}';").arg(pId, sImg));
    js.append(QString("document.head.appendChild(style);"));
//    qDebug() << "JS: " << js;
    webView->page()->runJavaScript(js);
}

void ExportHtml::save(QString filename)
{
    //try javascript
    webView->page()->runJavaScript("document.documentElement.outerHTML", [filename](const QVariant &v) {
        QFile file(filename);
        if(!file.open(QFile::WriteOnly | QFile::Text)){
            qDebug() << "Cannot create a file";
            return;
        }
        QTextStream stream(&file);
        stream << "<!DOCTYPE html>\n"<< v.toString();
        file.close();
    });
    //following not good on save file!!
    // webView->page()->save(filename, QWebEngineDownloadItem::CompleteHtmlSaveFormat); // BAD: have extra comment in <head> "<!-- saved from url ... -->", saved file size become large!!
    // webView->page()->save(filename, QWebEngineDownloadItem::MimeHtmlSaveFormat); // BAD wrong format
    // webView->page()->save(filename, QWebEngineDownloadItem::SingleHtmlSaveFormat); //BAD, did not show record
}

QString ExportHtml::imageToBase64(const QImage &image, const char *format)
{
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);

    // Save the image to the buffer
    image.save(&buffer, format);

    // Encode the buffer content to Base64
    QString base64String = byteArray.toBase64();

    return base64String;
}

void ExportHtml::setData(QList<TP *> tps,  QPixmap chat, QString pcs)
{
    // m_tpmgr = tpmgr;
    m_tps = tps;
    // m_tpplot =  tpplot;
    m_chat = chat;
    QJsonParseError error;
    QJsonDocument doc=QJsonDocument::fromJson(pcs.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError){
        m_pcs = doc.array();
    }else{
        qDebug() << "ExportHtml::setData Wrong format of pcs info: " << error.errorString();
    }
}

void ExportHtml::setRawFilenames(QStringList filenames)
{
    qDebug() << "setRawFilenames:" << filenames;
    m_iperf_raw_filenames = filenames;
}

void ExportHtml::setTestTime(QString time)
{
    m_testtime = time;
}

void ExportHtml::exporthtml()
{
    loadhtml(m_templatefile);// load template file
}

void ExportHtml::editTitleTag(QString title)
{
    // JavaScript to modify the content of the <h1> tag with id "title"
    QString js = "document.getElementById('title').innerHTML = '"+title+"';";
    webView->page()->runJavaScript(js);
}

void ExportHtml::updateTitle(QString title)
{
    QString js = "document.title='"+title+"'";
    webView->page()->runJavaScript(js);
}

void ExportHtml::onAddTag()
{   // TEST date
    QString tagId = "Config";
    QList<QString> list;
    list<< "TEST" << "<div class=\"dirTR\"></div>" << "127.0.0.1" <<
         QString::number(98) << QString::number(12) << "-P -t 10";

    AddDivRow(tagId, list);

    tagId = "HostInfo";
    list.clear();
    list << "192.168.0.23" << "Windows 11" << "22631" << "i219-v" << "v12.15.0.0";
    AddDivRow(tagId, list);

    save("/home/jimmy/SOFT/soft/myTools/QT/html/htmeditor/build/Qt_5_15_3-Debug/test.html");

}

void ExportHtml::onLoadFinished(bool isOk)
{
    // qDebug() << "onLoadFinished:" << isOk;
    if (isOk) {
        emit ready();
    }
}

void ExportHtml::procressData()
{
    // title
    updateTitle(m_testtime);
    editTitleTag(m_testtime);
    // TODO: DUT info

    QStringList ips;
    QList<QString> ls;// store managed ip in list
    // iperf test pairs
    foreach (TP *tp, m_tps) {
        ls.clear();

        QString s = tp->getServer();
        if (!ips.contains(s)) {
            ips.append(s);
        }
        ls.append(s);
        ls.append(dirToDiv(tp->getDirection()));
        QString c = tp->getClient();
        if (!ips.contains(c)) {
            ips.append(c);
        }
        ls.append(c);
        ls.append(tp->getThroughput());// TODO: Min/Max throughput?
        ls.append(tp->getLostRate());
        //Parameter: Delay time, iperf args
        QString para = "(Delay: "+ QString::number(tp->getDelaytime())+" sec)";
        para.append("<br>");
        para.append(m_iperfwrapper->toIperf3args(tp->getClientArgsMap()));
        ls.append(para);
        AddDivRow("Config", ls);
    }

    //throughput chart
    // QPixmap chat = m_tpplot->toPixmap(m_width, m_heigth);
    QString sImg = imageToBase64(m_chat.toImage());
    AddDivPng("tpchart", sImg);

    //host info
    QList<QString> hostls;
    QJsonParseError error;//= new QJsonParseError();
    QJsonDocument doc;
    // qDebug() << "procressData:" << m_pcs;
    for (const QJsonValue &value: qAsConst(m_pcs)) {
        hostls.clear();
        if (value.isString()) {
            QString pcinfo = value.toString();
            // qDebug() << "pcinfo:" << pcinfo;
            doc = QJsonDocument::fromJson(pcinfo.toUtf8(), &error);
            if (error.error == QJsonParseError::NoError){
                QJsonObject jObj = doc.object();
                QJsonObject jObjNet = jObj.value("Net").toObject();
                QJsonObject data;
                // qDebug() << "net interfaces: " << jObjNet.keys();
                foreach(const QString& key, jObjNet.keys()) {
                    data = jObjNet.value(key).toObject();
                    // qDebug() << "TODO NET address: " << data.value("address");
                    QJsonArray addrs = data.value("address").toArray();
                    for (QJsonArray::const_iterator it=addrs.constBegin(); it!=addrs.constEnd(); ++it) {
                        QJsonArray jAddr= it->toArray();
                        // qDebug() << "QJsonArray: " << jAddr;
                        for (int i=0;i< jAddr.count();i++){
                            QJsonValue v = jAddr.at(i);
                            if (ls.contains(v.toString())){
                                // if address is in managed ip list
                                hostls.append(v.toString());
                                QString hostname = jObj.value("HostName").toString();
                                if (jObj.value("MB_Model").toString().length()>0) {
                                    hostname = hostname + "<br>" + jObj.value("MB_Model").toString();
                                }
                                hostls.append(hostname);
                                hostls.append(jObj.value("OS").toString());
                                hostls.append(jObj.value("OSVer").toString());
                                QString ifname = key;
                                if (data.value("driverName").toString().length()>0){
                                    ifname = ifname + "<br>("+data.value("driverName").toString()+")";
                                }
                                hostls.append(ifname);
                                hostls.append(data.value("driverVersion").toString());
                                break;
                            }
                            QCoreApplication::processEvents(QEventLoop::AllEvents);
                        }
                        QCoreApplication::processEvents(QEventLoop::AllEvents);
                        if (hostls.count()>0){
                            break;
                        }
                    }
                    QCoreApplication::processEvents(QEventLoop::AllEvents);
                    if (hostls.count()>0){
                        break;
                    }
                }
            }else {
                qDebug() << " ERROR: " << error.errorString();
            }
        }else{
            qDebug() << "TODO QJsonArray value: " << value;
        }
        if (hostls.count()>0){
            // qDebug() << "hostls.count: " << hostls.count();
            AddDivRow("HostInfo", hostls);
        }
    }

    //iperf log raw data
    if(m_iperf_raw_filenames.length()>0){
        for ( const auto& filename : qAsConst(m_iperf_raw_filenames)){
            QFileInfo fileInfo(filename);
            QString filenameonly(fileInfo.fileName());
            QString rawdata;
            QFile datafile(filename);
            if (!datafile.open(QIODevice::ReadOnly)) {
                rawdata = "Can not open file "+ filename;
            }else{
                rawdata = datafile.readAll();
                datafile.close();
                // qDebug() << "filename:" << filenameonly <<" rawdata:" << rawdata;
            }
            AddRawData("RawData",filenameonly, rawdata);
            QCoreApplication::processEvents(QEventLoop::AllEvents);
        }
    }
    save(m_savefile);
}

void ExportHtml::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F12) {
        // Toggle the developer tools window
        // if (!devTools->isVisible()) {
        //     webView->page()->setDevToolsPage(devTools->page());  // Link dev tools to main page
        //     // devTools->show();
        //     devTools->showMaximized();
        // } else {
        //     devTools->close();
        // }
    } else {
        QWidget::keyPressEvent(event);  // Default behavior for other keys
    }
}

QString ExportHtml::dirToDiv(QString dir)
{
    if (dir.contains("Tx")){
        return "<div class=\"dirTx\"></div>";
    } else if (dir.contains("Rx")){
        return "<div class=\"dirRx\"></div>";
    } else if (dir.contains("TR")){
        return "<div class=\"dirTR\"></div>";
    } else {
        return "<div class=\"dirRT\"></div>";
    }
}
