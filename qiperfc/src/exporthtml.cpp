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

ExportHtml::ExportHtml(QString templatefile, QString savefile,int width, int heigth,
                       QWidget *parent)
    : QWidget{parent}, m_templatefile(templatefile), m_savefile(savefile),
    m_width(width), m_heigth(heigth)
{
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
    // qDebug() << "JS: " << js;
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

void ExportHtml::AddDivHostInfo(QString pId)
{
    Q_UNUSED(pId)
    // add host Info list
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
        stream << v.toString();
        file.close();
    });

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

void ExportHtml::setData(TPMgr *tpmgr, TPPlot *tpplot, QString pcs)
{
    m_tpmgr = tpmgr;
    m_tpplot =  tpplot;
    QJsonParseError error;
    QJsonDocument doc=QJsonDocument::fromJson(pcs.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError){
        m_pcs = doc.array();
    }else{
        qDebug() << "ExportHtml::setData Wrong format: " << error.errorString();
    }
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
    QList<QString> ls;
    // iperf test pairs
    QList<TP *> tps= m_tpmgr->getChilds(false);
    foreach (TP *tp, tps) {
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
        ls.append(m_iperfwrapper->toIperf3args(tp->getClientArgsMap()));
        AddDivRow("Config", ls);
    }

    //throughput chart
    QPixmap chat = m_tpplot->toPixmap(m_width, m_heigth);
    QString sImg = imageToBase64(chat.toImage());
    AddDivPng("tpchart", sImg);

    //host info
    // QJsonParseError *error= new QJsonParseError();
    // QJsonDocument doc = QJsonDocument();
    // qDebug() << "pcs:" << m_pcs;
    // for(QJsonArray::const_iterator it=m_pcs.constBegin(); it!=m_pcs.constEnd(); ++it){
    qDebug() << "procressData:" << m_pcs;
    // foreach (const QJsonValue &value, m_pcs) {
    for (const QJsonValue &value: qAsConst(m_pcs)) {
        if (value.isObject()) {
            QJsonObject jObj = value.toObject();
            qDebug() << "jObj.isEmpty:" << jObj.isEmpty();
            // qDebug() << "CPU: " <<  jObj.value("CPU").toString();
            // qDebug() << "MB_Model: " <<  jObj.value("MB_Model").toString();

            // qDebug() << "OS: " << jObj.value("OS").toString();
            // qDebug() << "OSVer: " << jObj.value("OSVer").toString();
            qDebug() << "NET: " << jObj.value("Net").toString();
        }else{
            qDebug() << "TODO QJsonArray value: " << value;
        }

    }
    // for(int i = 0; i < m_pcs.size(); ++i) {
    //     qDebug() << "m_pcs[i]: " <<    m_pcs[i].isString();//.isArray();//.isObject();// String??
    //     // QJsonObject jObj = m_pcs[i].toObject();
    //     qDebug() << "jObj:" << m_pcs[i];
    //     doc.fromJson(m_pcs[i].toString().toUtf8(), error);
    //     if (error->error == QJsonParseError::NoError) {
    //         QJsonObject jObj = doc.object();

    //         qDebug() << "CPU: " <<  jObj.value("CPU").toString();
    //         qDebug() << "MB_Model: " <<  jObj.value("MB_Model").toString();

    //         qDebug() << "OS: " << jObj.value("OS").toString();
    //         qDebug() << "OSVer: " << jObj.value("OSVer").toString();
    //     }else{
    //         qDebug() << "ExportHtml::procressData: Wrong format of m_pcs[i]: " << m_pcs[i].toString().toUtf8() << " ERROR:" << error->errorString() ;
    //         break;
    //     }

        // QJsonObject jNet = jObj.value("Net").toObject();
        // QString strNet = QJsonDocument(jNet).toJson(QJsonDocument::Compact);
        // qDebug() << "strNet: " << strNet;
        // jNet.
        //doc.setObject(jObj);
        //qDebug() << "PCS:" << doc.toJson(QJsonDocument::Compact);
    // }
    //raw data
    save(m_savefile);
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
