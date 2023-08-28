#include "endpointtype.h"

EndPointType::EndPointType(QObject *parent)
    : QObject(parent)
{
    m_types << "Unknown" << "Windows" << "Linux" << "MacOS" << "Android" << "iOS" << "FreeBSD";

}

QString EndPointType::getTypeString(Type tpy)
{
    return m_types.at(tpy);
}
