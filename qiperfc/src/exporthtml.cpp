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
    loadhtml(m_templatefile);
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
        js.append(QString("tdTag%1.classList.add('table-%2');").arg(idx).arg(td));
        js.append(QString("tdTag%1.innerHTML = '%2';").arg(idx).arg(value));
        js.append(QString("trTag.appendChild(tdTag%1);").arg(idx));
        idx++;
    }
    js.append(QString("pTag.appendChild(trTag);"));
    // qDebug() << "JS: " << js;
    webView->page()->runJavaScript(js);
}

void ExportHtml::AddDivPng(QString pId, QString sImg)
{
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
display: block;}';").arg(pId).arg(sImg));
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

void ExportHtml::setData(TPMgr *tpmgr, TPPlot *tpplot, QJsonArray pcs)
{
    m_tpmgr = tpmgr;
    m_tpplot =  tpplot;
    m_pcs = pcs;
}

void ExportHtml::editTitleTag()
{
    // JavaScript to modify the content of the <h1> tag with id "title"
    QString js = "document.getElementById('title').innerHTML = 'Hello, Qt WebEngine!';";
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
    qDebug() << "onLoadFinished:" << isOk;
    // m_ok = isOk;
    if (isOk) {
        emit ready();
    }
}

void ExportHtml::procressData()
{
    QList<QString> ls;
    QList<TP *> tps= m_tpmgr->getChilds();
    foreach (TP *tp, tps) {
        ls.clear();
        ls.append(tp->getServer());
        ls.append(dirToDiv(tp->getDirection()));
        ls.append(tp->getClient());
        ls.append(tp->getThroughput());
        ls.append(tp->getLostRate());
        //ls.append("");
        ls.append(m_iperfwrapper->toIperf3args(tp->getClientArgsMap()));// TODO, convert to iperf args
        AddDivRow("Config", ls);
    }

    //chart
    QPixmap chat = m_tpplot->toPixmap(m_width, m_heigth);
    QString sImg = imageToBase64(chat.toImage());
    AddDivPng("tpchart", sImg);
    //host info
    for(QJsonArray::const_iterator it=m_pcs.constBegin(); it!=m_pcs.constEnd(); ++it){
        QJsonObject jObj = it->toObject();
        qDebug() << "PCS:" << jObj;
    }
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
