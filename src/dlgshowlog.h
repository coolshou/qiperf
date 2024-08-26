#ifndef DLGSHOWLOG_H
#define DLGSHOWLOG_H

#include <QDialog>
#include "filewatcher.h"

namespace Ui {
class DlgShowLog;
}

class DlgShowLog : public QDialog
{
    Q_OBJECT

public:
    explicit DlgShowLog(const QString &filePath=nullptr,QWidget *parent = nullptr);
    ~DlgShowLog() override;
    void setLogFile(const QString &filePath);

public slots:
    void appendNewLine(QString line);

protected:
    void changeEvent(QEvent *e) override;
private slots:
    void onClear(bool checked);
private:
    Ui::DlgShowLog *ui;
    FileWatcher *m_filewatcher;

};

#endif // DLGSHOWLOG_H
