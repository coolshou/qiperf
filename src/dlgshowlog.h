#ifndef DLGSHOWLOG_H
#define DLGSHOWLOG_H

#include <QDialog>
#include <QKeyEvent>
#include <QEvent>

#include "filewatcher.h"

namespace Ui {
class DlgShowLog;
}

class DlgShowLog : public QDialog
{
    Q_OBJECT

public:
    explicit DlgShowLog(QWidget *parent = nullptr);
    explicit DlgShowLog(const QString &filePath=nullptr,QWidget *parent = nullptr);
    ~DlgShowLog() override;
    void setLogFile(const QString &filePath);

public slots:
    void appendNewLine(QString line);
    void doFind();
protected:
    void changeEvent(QEvent *e) override;
    void keyPressEvent(QKeyEvent * event) override;

private slots:
    void onClear(bool checked);
    void onPrev(bool checked);
    void onNext(bool checked);

private:
    Ui::DlgShowLog *ui;
    FileWatcher *m_filewatcher;
    void init();

};

#endif // DLGSHOWLOG_H
