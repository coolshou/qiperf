#include "serialview.h"
#include "ui_serialview.h"

SerialView::SerialView(QString title,
                       QString fontname, QString fontstyle, int fontsize,
                       QWidget *parent)
    : AbstractView(parent)
    , ui(new Ui::SerialView), m_title(title)
{
    ui->setupUi(this);
    m_currentport = new TcpUdpPort(this);
    m_currentport->setVisible(false); // no need to show TcpUdpPort's UI
    m_termialview = new TerminalView(fontname, fontstyle, fontsize, parent);
    setFocusProxy(m_termialview);
    ui->vLayout->addWidget(m_termialview);

    // terminal input data => write to m_currentport (TcpUdpPort)
    connect(m_termialview, &TerminalView::transmitData, this, &SerialView::writePortData);
    // m_currentport data ready to read => readall and show on m_termialview
    connect(m_currentport, &AbstractPort::readyRead, this, &SerialView::readPortData);
}

SerialView::~SerialView()
{
    qDebug() << "~SerialView";
    delete ui;
}

QString SerialView::title()
{
    return m_title;
}

QString SerialView::iid()
{
    return "Serial";
}

void SerialView::setConfig(QString serveraddress, int portnumber, QString protocol)
{
    m_currentport->setConfig(serveraddress, portnumber, protocol);
    m_currentport->open();
}

void SerialView::setLogFile(bool logtofile, QString logfilename, bool logtimestemp, QString timestempformat)
{
    //setup log to file
    m_termialview->setLogFile(logtofile, logfilename, logtimestemp, timestempformat);
}

void SerialView::onCopy()
{
    if(m_termialview){
        m_termialview->onCopy();
    }
}

void SerialView::onPaste()
{
    if(m_termialview){
        m_termialview->onPaste();
    }
}

void SerialView::onDelete()
{
    if(m_termialview){
        m_termialview->onDelete();
    }
}

void SerialView::onCopyText()
{
    if(m_termialview){
        m_termialview->onCopyText();
    }
}

void SerialView::readPortData()
{
    if (m_pause == false) {
        QByteArray array = m_currentport->readAll();
        if (!array.isEmpty()) {
            m_rxCount += array.length();
            m_termialview->receiveData(array);
            // m_toolBoxs->receiveData(array);
        }
    }
}

void SerialView::writePortData(const QByteArray &array)
{
    m_txCount += array.length();
    m_currentport->write(array);
}

void SerialView::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void SerialView::closeEvent(QCloseEvent *event)
{
    qDebug() << "TODO SerialView::closeEvent:" << event;
    emit closed(m_title);
}
