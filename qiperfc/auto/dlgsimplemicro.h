#ifndef DLGSIMPLEMICRO_H
#define DLGSIMPLEMICRO_H

#include <QDialog>
#include <QThread>
#include <QMenu>
#include <QAction>
#include <QPoint>

#include "simpleworker.h"

namespace Ui {
class DlgSimpleMicro;
}

class DlgSimpleMicro : public QDialog
{
    Q_OBJECT

public:
    explicit DlgSimpleMicro(QWidget *parent = nullptr);
    ~DlgSimpleMicro() override;
public slots:
    void onStart(bool checked);
    void onLoadFile(QString idx, QString filename);

signals:
    void loadfile(QString idx, QString filename);
protected:
    void changeEvent(QEvent *e);
    void initRightMenu();
private slots:
    void showContextMenu(const QPoint &pos);
    void onInsert(bool checked);
    void onDelete(bool checked);
    void onCopy(bool checked);
    void onPaste(bool checked);
private:
    Ui::DlgSimpleMicro *ui;
    SimpleWorker *m_sworker;
    QThread *m_thread;
    QMenu *m_rightmenu;
    QAction *m_insertAction;
    QAction *m_deleteAction;
    QAction *m_copyAction;
    QAction *m_pasteAction;
};

#endif // DLGSIMPLEMICRO_H
