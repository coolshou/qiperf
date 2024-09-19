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

ExportHtml::ExportHtml(QString templatefile, QWidget *parent)
    : QWidget{parent}, m_templatefile(templatefile)
{
    QNetworkProxyFactory::setUseSystemConfiguration(false); // not use system proxy
    QVBoxLayout *layout = new QVBoxLayout(this);
    webView = new QWebEngineView(this);
    webView->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    QPushButton *addButton = new QPushButton("add Tag", this);
    layout->addWidget(webView);
    layout->addWidget(addButton);

    connect(addButton, &QPushButton::clicked, this, &ExportHtml::onAddTag);

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
    js.append("var trTag = document.createElement('div');trTag.classList.add('table-tr');");
    int idx=0;
    foreach (QString value, values) {
        js.append(QString("var tdTag%1 = document.createElement('div');").arg(idx));
        js.append(QString("tdTag%1.classList.add('table-td');").arg(idx));
        js.append(QString("tdTag%1.innerHTML = '%2';").arg(idx).arg(value));
        js.append(QString("trTag.appendChild(tdTag%1);").arg(idx));
        idx++;
    }
    js.append("pTag.appendChild(trTag);");
    // qDebug() << "JS: " << js;
    webView->page()->runJavaScript(js);
}

void ExportHtml::save(QString filename)
{
    webView->page()->save(filename, QWebEngineDownloadItem::CompleteHtmlSaveFormat);
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

void ExportHtml::processdata(TPMgr *tpmgr, TPPlot *m_tpplot)
{
    QList<QString> ls;
    QList<TP *> tps= tpmgr->getChilds();
    foreach (TP *tp, tps) {
        ls.append(tp->getServer());
        ls.append(tp->getDirection());
        ls.append(tp->getClient());
        ls.append(tp->getThroughput());
        ls.append(tp->getLostRate());
        ls.append(tp->getClientArgs());
    }
    AddDivRow("Config", ls);

}

void ExportHtml::editTitleTag()
{
    // JavaScript to modify the content of the <h1> tag with id "title"
    QString js = "document.getElementById('title').innerHTML = 'Hello, Qt WebEngine!';";
    webView->page()->runJavaScript(js);
}

void ExportHtml::onAddTag()
{
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
