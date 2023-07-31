#ifndef ENDPOINTTYPE_H
#define ENDPOINTTYPE_H

#include <QObject>

class EndPointType : public QObject
{
    Q_OBJECT
public:
    explicit EndPointType(QObject *parent = nullptr);

    enum Type{
        Unknown=0,
        Windows=1,
        Linux,
        MacOS,
        Android,
        iOS,
        FreeBSD
    };
    Q_ENUM(Type)

    QString getTypeString(EndPointType::Type tpy);

private:
    QStringList m_types;

};

#endif // ENDPOINTTYPE_H
