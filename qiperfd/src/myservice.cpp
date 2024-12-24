#include "myservice.h"
#include <QSysInfo>

#include <QDebug>
/*windows get service status
 sc query qiperfd
SERVICE_NAME: qiperfd
        TYPE               : 10  WIN32_OWN_PROCESS
        STATE              : 4  RUNNING
                                (STOPPABLE, NOT_PAUSABLE, ACCEPTS_SHUTDOWN)
        WIN32_EXIT_CODE    : 0  (0x0)
        SERVICE_EXIT_CODE  : 0  (0x0)
        CHECKPOINT         : 0x0
        WAIT_HINT          : 0x0
 sc start qiperfd
 sc stop qiperfd
 ==>TODO sc restart

*/
/* Linux get serive status
systemctl status qiperfd

qiperfd.service - quick iperf daemon service
     Loaded: loaded (/usr/lib/systemd/system/qiperfd.service; enabled; vendor preset: enabled)
     Active: active (running) since Thu 2024-08-08 08:58:34 CST; 4 days ago
   Main PID: 1370 (qiperfd)
      Tasks: 1 (limit: 18654)
     Memory: 2.2M
        CPU: 10.429s
     CGroup: /system.slice/qiperfd.service
             └─1370 /opt/qiperf/bin/qiperfd

systemctl start qiperfd
systemctl stop qiperfd
systemctl restart qiperfd
*/


MyService::MyService(QIperfd *qiperfd, QObject *parent)
    : QObject{parent}
{
    m_qiperfd = qiperfd;
//    m_interface="";
}

QString MyService::getOS()
{
    return QSysInfo::productType();
}

int MyService::addIperfServer(QString refrow, int version, uint port, QString bindHost)
{
    return m_qiperfd->addIperfServer(refrow, version, port, bindHost);
}

int MyService::addIperfClient(QString refrow, int version, uint port, QString Host, QString iperfargs)
{
    return m_qiperfd->addIperfClient(refrow, version, port, Host, iperfargs);
}

void MyService::start(int idx)
{
    return m_qiperfd->start(idx);
}

void MyService::startAll()
{
    return m_qiperfd->startAll();
}

void MyService::stop(int idx)
{
    return m_qiperfd->stop(idx);
}

void MyService::stopAll()
{
    return m_qiperfd->stopAll();
}

void MyService::setManagerInterface(QString ifname)
{
//    m_interface = interface;
   emit sig_setManagerInterface(ifname);
}

QString MyService::getManagerInterface()
{
   return m_qiperfd->getManagerInterface();
//    return m_interface;
}
