#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <QObject>
#include <QFileSystemWatcher>

class FileWatcher : public QObject
{
    Q_OBJECT
public:
    explicit FileWatcher(const QString &filePath, QObject *parent = nullptr);

signals:
    void onNewLine(QString line);

private slots:
    void onFileChanged(const QString &path);
    void readFile();

private:
    void log(QString message);
    QString m_filePath;
    QFileSystemWatcher *m_fileWatcher;
    qint64 m_filepos;
};

#endif // FILEWATCHER_H
