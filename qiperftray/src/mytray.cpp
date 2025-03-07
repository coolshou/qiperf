#include "mytray.h"
#include "qobjectdefs.h"

#include <QIcon>
#include <QMenu>
#include <QAction>
#include <QStandardPaths>
#include "comm.h"

#include <QDebug>

MyTray::MyTray(QObject *parent):QObject(parent)
{
    QIcon icon(":/qiperf");
    QMenu *trayIconMenu = new QMenu();
    QAction *viewWindow = new QAction("Show", this);
    connect(viewWindow, &QAction::triggered, this, &MyTray::sigShow);
    QAction *quitAction = new QAction("Quit", this);
    connect(quitAction, &QAction::triggered, this, &MyTray::sigQuit);
    trayIconMenu->addAction(viewWindow);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayicon = new QSystemTrayIcon();
    trayicon->setIcon(icon);
    trayicon->setContextMenu(trayIconMenu);
    trayicon->setToolTip("qiperf daemon");

    connect(trayicon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    trayicon->show();
    QString tmp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString qiperfdlog = tmp + "/qiperf/"+QString(QIPERFD_NAME)+".log" ;

}

bool MyTray::supportsMessages()
{
    return trayicon->supportsMessages();
}

bool MyTray::isVisible() const
{
    return trayicon->isVisible();
}

void MyTray::hideIconTray()
{
    trayicon->hide();
}
void MyTray::showIconTray()
{
    trayicon->show();
}

void MyTray::showMessage(const QString &title, const QString &message, QSystemTrayIcon::MessageIcon icon, int millisecondsTimeoutHint)
{
    trayicon->showMessage(title, message, icon, millisecondsTimeoutHint);
}

void MyTray::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason){
    case QSystemTrayIcon::Trigger:
        emit sigIconActivated();
        break;

    default:
        break;
    }
}
