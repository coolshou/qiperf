#ifndef MYTRAY_H
#define MYTRAY_H

#include <QSystemTrayIcon>
#include <QObject>

class MyTray : public QObject
{
    Q_OBJECT
public:
    explicit MyTray(QObject *parent = nullptr);

signals:
    void sigIconActivated();
    void sigShow();
    void sigQuit();

public slots:
    void hideIconTray();
    void showIconTray();

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    QSystemTrayIcon *trayicon;

};

#endif // MYTRAY_H
