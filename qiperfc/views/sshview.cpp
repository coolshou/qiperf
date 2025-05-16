#include "sshview.h"
#include "ui_sshview.h"

SSHView::SSHView(QString title, QWidget *parent)
    : AbstractView(parent)
    , ui(new Ui::SSHView), m_title(title)
{
    ui->setupUi(this);
    m_currentport = new TcpUdpPort(this);
    m_currentport->setVisible(false); // no need to show TcpUdpPort's UI
    m_termialview = new TerminalView(parent);
    ui->vLayout->addWidget(m_termialview);

    // terminal input data => write to m_currentport (TcpUdpPort)
    connect(m_termialview, &TerminalView::transmitData, this, &SSHView::writePortData);
    // m_currentport data ready to read => readall and show on m_termialview
    connect(m_currentport, &AbstractPort::readyRead, this, &SSHView::readPortData);

}

SSHView::~SSHView()
{
    delete ui;
}

void SSHView::setConfig(QString serveraddress, int portnumber, QString protocol)
{

}

void SSHView::setLogFile(bool logtofile, QString logfilename, bool logtimestemp, QString timestempformat)
{

}

QString SSHView::iid()
{
    return "SSH";
}

QString SSHView::title()
{
    return m_title;
}

void SSHView::changeEvent(QEvent *e)
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

void SSHView::readPortData()
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

void SSHView::writePortData(const QByteArray &array)
{
    m_txCount += array.length();
    m_currentport->write(array);
}
