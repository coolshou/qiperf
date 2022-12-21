#include "mytray.h"
#include "qobjectdefs.h"

#include <QIcon>
#include <QMenu>
#include <QAction>

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

    connect(trayicon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    trayicon->show();
}

void MyTray::hideIconTray()
{
    trayicon->hide();
}
void MyTray::showIconTray()
{
    trayicon->show();
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
