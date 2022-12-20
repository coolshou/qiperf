#ifndef QIPERFD_H
#define QIPERFD_H

#include <QObject>
#include "pipeserver.h"

class QIperfd : public QObject
{
    Q_OBJECT
public:
    explicit QIperfd(QObject *parent = nullptr);
public slots:
    void onNewMessage(int idx, const QString msg);
signals:
private:
    PipeServer *m_pserver;

};

#endif // QIPERFD_H
