#include "serialview.h"
#include "ui_serialview.h"

SerialView::SerialView(QString title, QWidget *parent)
    : AbstractView(parent)
    , ui(new Ui::SerialView), m_title(title)
{
    ui->setupUi(this);
    m_serialport = new SerialPort();

    m_termialview = new TerminalView(parent);
    ui->vLayout->addWidget(m_termialview);

    connect(m_termialview, &TerminalView::transmitData, this, &SerialView::writePortData);
    // connect(m_termialview, &TerminalView::sendMessage, this, &ViewManager::dispatchMessage);
    connect(m_serialport, &SerialPort::readyRead, this, &SerialView::readPortData);

}

SerialView::~SerialView()
{
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

void SerialView::readPortData()
{
    if (m_pause == false) {
        QByteArray array = m_serialport->readAll();

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
    m_serialport->write(array);
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
