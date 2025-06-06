#ifndef CYNTEC_H
#define CYNTEC_H

#include <QObject>
#include "aip.h"

class Cyntec : public AiP
{
    Q_OBJECT
public:
    explicit Cyntec(QObject *parent = nullptr);
};

#endif // CYNTEC_H
