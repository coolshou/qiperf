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
    QString m_filePath;
    QFileSystemWatcher m_fileWatcher;
};

#endif // FILEWATCHER_H
