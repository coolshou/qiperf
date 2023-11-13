#include "icmpping.h"

#if defined(Q_OS_WIN32)
#include "winsock2.h"
#include "iphlpapi.h"
#include "icmpapi.h"
#endif

#include <QDebug>

IcmpPing::IcmpPing(QObject *parent)
    : QObject{parent}
{
//QString h;
//h.toStdString().
}

bool IcmpPing::pingHost(const QString &hostname, int count)
{
#if defined(Q_OS_WIN32)
    // We declare variables
    HANDLE hIcmpFile;                       // Handler
    unsigned long ipaddr = INADDR_NONE;     // Destination address
    DWORD dwRetVal = 0;                     // Number of replies
    char SendData[32] = "Data Buffer";      // The buffer data being sent
    LPVOID ReplyBuffer = NULL;              // buffer replies
    DWORD ReplySize = 0;                    // Buffer Size responses

    // Set the IP-address of the field qlineEdit
    ipaddr = inet_addr(hostname.toStdString().c_str());
    hIcmpFile = IcmpCreateFile();   // create a handler

    // Select the buffer memory responses
    ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
    ReplyBuffer = (VOID*) malloc(ReplySize);

    // Call the ICMP echo request function
    dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData),
                NULL, ReplyBuffer, ReplySize, 1000);

    // We create a row in which we write the response message
    QString strMessage = "";

    if (dwRetVal != 0) {
        // The structure of the echo response
        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
        struct in_addr ReplyAddr;
        ReplyAddr.S_un.S_addr = pEchoReply->Address;

        strMessage += "Sent icmp message to " + hostname + "\n";
        if (dwRetVal > 1) {
            strMessage += "Received " + QString::number(dwRetVal) + " icmp message responses \n";
            strMessage += "Information from the first response: ";
        }
        else {
            strMessage += "Received " + QString::number(dwRetVal) + " icmp message response \n";
            strMessage += "Information from the first response: ";
        }
            strMessage += "Received from ";
            strMessage += inet_ntoa( ReplyAddr );
            strMessage += "\n";
            strMessage += "Status = " + pEchoReply->Status;
            strMessage += "Roundtrip time = " + QString::number(pEchoReply->RoundTripTime) + " milliseconds \n";
    } else {
        strMessage += "Call to IcmpSendEcho failed.\n";
        strMessage += "IcmpSendEcho returned error: ";
        strMessage += QString::number(GetLastError());
    }
    qDebug() << strMessage << Qt::endl;
    //ui->textEdit->setText(strMessage); // Display information about the received data
    free(ReplyBuffer); // frees memory
#endif
#if defined(Q_OS_LINUX)
    qDebug() << "Ping:" << hostname << " , count:" << count << Qt::endl;
    /*
    oping *ping = oping_new();
    oping_set_host(ping, hostname.toStdString().c_str());

    for (int i = 0; i < count; ++i)
    {
        if (oping_send(ping) == 1)
        {
            const oping_reply *reply = oping_recv(ping);
            if (reply && reply->response)
            {
                emit pingSuccess(reply->response_time);
            }
            else
            {
                emit pingFailed();
            }
        }
        else
        {
            emit pingFailed();
        }
    }

    oping_free(ping);
    */
#endif
    return true;
}
