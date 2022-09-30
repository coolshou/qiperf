#include "agent.h"
#include <QNetworkDatagram>

#include <QDebug>

Agent::Agent(QObject *parent, const QHostAddress& outgoingIP,
             unsigned int outgoingPort, bool acceptmsg):
    QObject(parent),
    udpSocket(new QUdpSocket(this)),
    outputIPAddress(outgoingIP),
    outputPort(outgoingPort),
    m_acceptmsg(acceptmsg)
{
    if (m_acceptmsg){
        QObject::connect(udpSocket,&QUdpSocket::readyRead,this,&Agent::processData);
    }
    initialize();
    timer = new QTimer();
    timer->setInterval(5000);
    QObject::connect(timer, &QTimer::timeout, this, &Agent::broadcastDatagram);
//    udpSocket->setMulticastInterface();
    timer->start();
//    qDebug() << "if: " << udpSocket->multicastInterface().humanReadableName() << Qt::endl;

}
void Agent::processData()
{
    while (udpSocket->hasPendingDatagrams())
    {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        //Process valid packets
        if(!datagram.isNull())
            emit receivedMessage(QString::fromStdString(datagram.data().toStdString()));
    }
}
void Agent::sendMessage(QString message)
{
    //Send single message
    if (0){
        //require bind
        QNetworkDatagram ndatagram;
        ndatagram.setData(QByteArray::fromStdString(message.toStdString()));
        ndatagram.setInterfaceIndex(udpSocket->multicastInterface().index());
        ndatagram.setDestination(outputIPAddress,
                                static_cast<quint16>(outputPort));
        udpSocket->writeDatagram(ndatagram);
    }else{
        QByteArray datagram =message.toUtf8();
        udpSocket->writeDatagram(datagram, QHostAddress::Broadcast, static_cast<quint16>(outputPort));
    }
}

void Agent::updateIpaddress(QString ipaddress, int port)
{
    qDebug() << "updateIpaddress:" << ipaddress << " port:" << QString::number(port) << Qt::endl;
    outputIPAddress = QHostAddress(ipaddress);
    outputPort = port;
}

void Agent::initialize()
{
    //Unbind the socket if currently bound
    if(udpSocket->state() == QAbstractSocket::BoundState)
        udpSocket->close();

    //Bind the socket to the specified input address and port
//    udpSocket->bind(inputIPAddress,
//                    static_cast<quint16>(inputPort));

//    udpSocket->setMulticastInterface(networkAdapter);               //Set the outgoing interface
//    udpSocket->joinMulticastGroup(inputIPAddress,
//                                  udpSocket->multicastInterface()); //Join incoming multicast group

//    emit adapterUpdated(udpSocket->multicastInterface().humanReadableName());
}

void Agent::broadcastDatagram()
{
//    qDebug() << "broadcastDatagram" << Qt::endl;
    sendMessage("{'RPCIP':'" +  outputIPAddress.toString()+ "'"+
                ",'RPCPORT':"+ QString::number(outputPort+1)+"}");
}

bool Agent::isValidNetworkAdapter(const QNetworkInterface &adapter)
{
    //Basic check
    if(!adapter.isValid())
        return false;

    //Has to be up and running
    if(!(adapter.flags() & QNetworkInterface::InterfaceFlag::IsUp))
        return false;

    //Has to support multicast
    if(!(adapter.flags() & QNetworkInterface::InterfaceFlag::CanMulticast))
        return false;

    //Additional filters (bluetooth / VM)
    if(adapter.humanReadableName().toUpper().contains("BLUETOOTH") || adapter.humanReadableName().toUpper().contains("VM")
        ||adapter.humanReadableName().toUpper().contains("DOCKER")||adapter.humanReadableName().toUpper().contains("LXCB")
        ||adapter.humanReadableName().toUpper().contains("VIRBR")||adapter.humanReadableName().toUpper().contains("VBOXNET"))
        return false;

    return true;
}

bool Agent::isValidNetworkAdapter(QString adapterName)
{
    return Agent::getNetworkAdapter(adapterName).isValid();
}

QNetworkInterface Agent::getNetworkAdapter(QString adapterName)
{
    //Find a valid network interface of the given name if it exists
    foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
    {
        if(Agent::isValidNetworkAdapter(interface))
        {
            bool match = (interface.humanReadableName().compare(adapterName) == 0);
            if(match)
                return interface;
        }
    }

    return QNetworkInterface();
}
