#ifndef SIMPLEWORKER_H
#define SIMPLEWORKER_H

#include <QObject>

class SimpleWorker : public QObject
{
    Q_OBJECT
public:
    explicit SimpleWorker(QStringList files, QObject *parent = nullptr);
    void run();

signals:
    void loadfile(QString idx, QString filename);

private:
    QStringList m_files;
};

#endif // SIMPLEWORKER_H
