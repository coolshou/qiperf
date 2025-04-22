#include "serialview.h"
#include "ui_serialview.h"

SerialView::SerialView(QString title, QWidget *parent)
    : AbstractView(parent)
    , ui(new Ui::SerialView), m_title(title)
{
    ui->setupUi(this);
    m_currentport = new TcpUdpPort(this);
    m_currentport->setVisible(false); // no need to show TcpUdpPort's UI
    m_termialview = new TerminalView(parent);
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
    //TODO:title
    return m_title;
}

QString SerialView::iid()
{
    //TODO: iid
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

void SerialView::readPortData()
{
    if (m_pause == false) {
        QByteArray array = m_currentport->readAll();
        // QByteArray array = m_serialport->readAll();

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
    // m_serialport->write(array);
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
