#ifndef SSHTASK_H
#define SSHTASK_H

#include <QObject>

#include "sshdeviceshell.h"
#include "../virtualdevicetcp.h"

class SSHTask : public QObject
{
    Q_OBJECT
public:
    explicit SSHTask(QObject *parent = nullptr);

signals:
};

#endif // SSHTASK_H
