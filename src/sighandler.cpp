#include "sighandler.h"

#if defined(Q_OS_LINUX)

#include <QDebug>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

int SigHandler::sigintFd[2];
int SigHandler::sighupFd[2];
int SigHandler::sigtermFd[2];

SigHandler::SigHandler(QObject *parent)
        : QObject(parent)
{
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigintFd))
        qFatal("Couldn't create INT socketpair");
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sighupFd))
       qFatal("Couldn't create HUP socketpair");
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigtermFd))
       qFatal("Couldn't create TERM socketpair");
    snInt = new QSocketNotifier(sigintFd[1], QSocketNotifier::Read, this);
    connect(snInt, SIGNAL(activated(QSocketDescriptor)), this, SLOT(handleSigInt()));
    snHup = new QSocketNotifier(sighupFd[1], QSocketNotifier::Read, this);
    connect(snHup, SIGNAL(activated(QSocketDescriptor)), this, SLOT(handleSigHup()));
    snTerm = new QSocketNotifier(sigtermFd[1], QSocketNotifier::Read, this);
    connect(snTerm, SIGNAL(activated(QSocketDescriptor)), this, SLOT(handleSigTerm()));


}

SigHandler::~SigHandler()
{
    delete snHup;
    delete snTerm;
    delete snInt;
}

void SigHandler::intSignalHandler(int unused)
{
    Q_UNUSED(unused)
    char a = 1;
    ::write(sigintFd[0], &a, sizeof(a));

}

void SigHandler::hupSignalHandler(int unused)
{
    Q_UNUSED(unused)
    char a = 1;
    ::write(sighupFd[0], &a, sizeof(a));
}

void SigHandler::termSignalHandler(int unused)
{
    Q_UNUSED(unused)
    char a = 1;
    ::write(sigtermFd[0], &a, sizeof(a));
}

void SigHandler::handleSigInt()
{
    snInt->setEnabled(false);
    char tmp;
    ::read(sigtermFd[1], &tmp, sizeof(tmp));

    // do Qt stuff
    qDebug() << "handleSigInt" << Qt::endl;
    emit sigINT();
    snTerm->setEnabled(true);

}

void SigHandler::handleSigTerm()
{
    snTerm->setEnabled(false);
    char tmp;
    ::read(sigtermFd[1], &tmp, sizeof(tmp));

    // do Qt stuff
    qDebug() << "handleSigTerm" << Qt::endl;
    emit sigTERM();
    snTerm->setEnabled(true);
}

void SigHandler::handleSigHup()
{
    snHup->setEnabled(false);
    char tmp;
    ::read(sighupFd[1], &tmp, sizeof(tmp));

    // do Qt stuff
    qDebug() << "handleSigHup" << Qt::endl;
     emit sigHUP();
    snHup->setEnabled(true);
}

#endif
