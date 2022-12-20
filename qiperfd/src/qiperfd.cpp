#include "qiperfd.h"
#include "../src/comm.h"

#include <QString>

#include <QDebug>
QIperfd::QIperfd(QObject *parent)
    : QObject{parent}
{
    //GUI interaction interface
    m_pserver=new PipeServer(QIPERFD_NAME, NULL);
    connect(m_pserver, SIGNAL(newMessage(int, QString)), this, SLOT(onNewMessage(int, QString)));
    if (m_pserver->init()){
        qDebug() << "PipeServer start fail" << Qt::endl;
    }
    //iperf control interface
    //system service manager
}

void QIperfd::onNewMessage(int idx, const QString msg)
{
    qDebug() << "TODO: handle new message:("<<idx<<")" <<  (msg) << Qt::endl;
    if (QString::compare(msg, CMD_STATUS, Qt::CaseInsensitive)==0){
        //get current all iperf status
        qDebug() << "TODO: send current status back to GUI" << Qt::endl;
        m_pserver->send_MessageBack(idx, "message back to client?");
    }
}
