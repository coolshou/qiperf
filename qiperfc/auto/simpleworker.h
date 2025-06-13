#ifndef SIMPLEWORKER_H
#define SIMPLEWORKER_H

#include <QObject>
#include <QMap>

class SimpleWorker : public QObject
{
    Q_OBJECT
public:
    explicit SimpleWorker(QStringList files, QString savepath, QObject *parent = nullptr);
    void run();
    void setStop();
    void setTP(QString key, double value);
public slots:
    void setTPLostRate(int idx, double value, double lostrate);
signals:
    void loadfile(QString idx, QString filename, QString savepath);
    void started();
    void progress(int value);
    void stoped();

private:
    QStringList m_files;
    QString m_savepath;
    bool m_stop;
    QMap<int, double> m_tpdata;
    QMap<int, double> m_lostratedata;
};

#endif // SIMPLEWORKER_H
