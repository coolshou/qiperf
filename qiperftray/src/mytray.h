#ifndef MYTRAY_H
#define MYTRAY_H

#include <QSystemTrayIcon>
#include <QObject>

class MyTray : public QObject
{
    Q_OBJECT
public:
    explicit MyTray(QObject *parent = nullptr);
    bool supportsMessages();
    bool isVisible() const;

signals:
    void sigIconActivated();
    void sigShow();
    void sigQuit();

public slots:
    void hideIconTray();
    void showIconTray();
    void showMessage(const QString &title, const QString &message,
                     QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information,
                     int millisecondsTimeoutHint = 10000);

private slots:
    void iconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    QSystemTrayIcon *trayicon;

};

#endif // MYTRAY_H
