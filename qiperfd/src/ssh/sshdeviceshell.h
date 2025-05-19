#ifndef SSHDEVICESHELL_H
#define SSHDEVICESHELL_H

#include <QObject>
#include "../virtualdevice.h"
#include <qssh/sshconnection.h>

class SSHDeviceShell : public VirtualDevice
{
    Q_OBJECT
public:
    explicit SSHDeviceShell(QObject *parent = nullptr);
};

#endif // SSHDEVICESHELL_H
