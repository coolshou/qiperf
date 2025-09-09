#ifndef OPTIMIZEDATA_H
#define OPTIMIZEDATA_H

#include <QDateTime>
#include <QString>

class OptimizeData
{
public:
    OptimizeData(QDateTime time=QDateTime()) : m_testdate(time),
        m_id(-1), m_mcs(-1.0), m_rssi(0.0), m_snr(0.0),
        m_cmName(""), m_cmId(-1), m_cmMcs(-1.0), m_cmRssi(0.0), m_cmSnr(0.0),
        m_ul(-1), m_dl(-1){}

    OptimizeData(QDateTime time, int id, double mcs, double rssi, double snr,
                 QString cmName, int cmId, double cmMcs, double cmRssi, double cmSnr)
        : m_testdate(time), m_id(id), m_mcs(mcs), m_rssi(rssi), m_snr(snr),
        m_cmName(cmName), m_cmId(cmId), m_cmMcs(cmMcs), m_cmRssi(cmRssi), m_cmSnr(cmSnr) {}

    bool isEmpty() const {
        return m_id <= 0;  // Define invalid ID as "empty"
    }

    // Getters
    QDateTime testdate() const { return m_testdate; }
    int id() const { return m_id; }
    double mcs() const { return m_mcs; }
    double rssi() const { return m_rssi; }
    double snr() const { return m_snr; }
    QString cmName() const { return m_cmName; }
    int cmId() const { return m_cmId; }
    double cmMcs() const { return m_cmMcs; }
    double cmRssi() const { return m_cmRssi; }
    double cmSnr() const { return m_cmSnr; }
    double ul() const { return m_ul; }
    double dl() const { return m_dl; }

    // Optional: setters if you want mutability
    void setTestDate(QDateTime t) { m_testdate = t; }
    void setId(int id) { m_id = id; }
    void setMcs(double mcs) { m_mcs = mcs; }
    void setRssi(double rssi) { m_rssi = rssi; }
    void setSnr(double snr) { m_snr = snr; }
    void setCmName(QString cmName) { m_cmName = cmName; }
    void setCmId(int cmId) { m_cmId = cmId; }
    void setCmMcs(double cmMcs) { m_cmMcs = cmMcs; }
    void setCmRssi(double cmRssi) { m_cmRssi = cmRssi; }
    void setCmSnr(double cmSnr) { m_cmSnr = cmSnr; }
    void setUl(double ul) { m_ul = ul; }
    void setDl(double dl) { m_dl = dl; }

private:
    QDateTime m_testdate;
    int m_id;
    double m_mcs;
    double m_rssi;
    double m_snr;
    QString m_cmName;
    int m_cmId;
    double m_cmMcs;
    double m_cmRssi;
    double m_cmSnr;
    double m_ul;
    double m_dl;
};

#endif //
