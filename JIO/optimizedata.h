#ifndef OPTIMIZEDATA_H
#define OPTIMIZEDATA_H

#include <QDateTime>
#include <QString>

class OptimizeData
{
public:
    OptimizeData(QDateTime time=QDateTime()) : m_testdate(time),
        m_id(-1), m_mcs(-1.0), m_rssi(0.0), m_snr(0.0),
        m_cName(""), m_cId(-1), m_cMcs(-1.0), m_cRssi(0.0), m_cSnr(0.0),
        m_ul(-1.0), m_dl(-1.0){}
    OptimizeData(QDateTime time, int id, QString cName, int cmId)
        : m_testdate(time), m_id(id), m_cName(cName), m_cId(cmId)
    {
        m_mcs = -1.0;
        m_rssi = 0.0;
        m_snr = 0.0;
        m_cMcs = -1.0;
        m_cRssi = 0.0;
        m_cSnr = 0.0;
        m_ul = -1.0;
        m_dl = -1.0;
    }
    OptimizeData(QDateTime time, int id, double mcs, double rssi, double snr,
                 QString cName, int cId, double cMcs, double cRssi, double cSnr,
                 double ul=0.0, double dl=0.0)
        : m_testdate(time), m_id(id), m_mcs(mcs), m_rssi(rssi), m_snr(snr),
        m_cName(cName), m_cId(cId), m_cMcs(cMcs), m_cRssi(cRssi), m_cSnr(cSnr),
        m_ul(ul), m_dl(dl) {}

    bool isEmpty() const {
        return m_id <= 0;  // Define invalid ID as "empty"
    }

    // Getters
    QDateTime testdate() const { return m_testdate; }
    int id() const { return m_id; }
    double mcs() const { return m_mcs; }
    double rssi() const { return m_rssi; }
    double snr() const { return m_snr; }
    QString cName() const { return m_cName; }
    int cId() const { return m_cId; }
    double cMcs() const { return m_cMcs; }
    double cRssi() const { return m_cRssi; }
    double cSnr() const { return m_cSnr; }
    double ul() const { return m_ul; }
    double dl() const { return m_dl; }

    // Optional: setters if you want mutability
    void setTestDate(QDateTime t) { m_testdate = t; }
    void setId(int id) { m_id = id; }
    void setMcs(double mcs) { m_mcs = mcs; }
    void setRssi(double rssi) { m_rssi = rssi; }
    void setSnr(double snr) { m_snr = snr; }
    void setCName(QString cName) { m_cName = cName; }
    void setCId(int cmId) { m_cId = cmId; }
    void setCMcs(double cMcs) { m_cMcs = cMcs; }
    void setCRssi(double cRssi) { m_cRssi = cRssi; }
    void setCSnr(double cSnr) { m_cSnr = cSnr; }
    void setUl(double ul) { m_ul = ul; }
    void setDl(double dl) { m_dl = dl; }

private:
    QDateTime m_testdate;
    int m_id;
    double m_mcs;
    double m_rssi;
    double m_snr;
    QString m_cName;
    int m_cId;
    double m_cMcs;
    double m_cRssi;
    double m_cSnr;
    double m_ul;
    double m_dl;
};

#endif //
