#ifndef DLGSIMPLEMICRO_H
#define DLGSIMPLEMICRO_H

#include <QDialog>
#include <QThread>
#include <QMenu>
#include <QAction>
#include <QPoint>
#include <QClipboard>
#include <QTableWidget>

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
    void onStop(bool checked);
    void onSelectSavePath(bool checked);
    void onLoad(bool checked);
    void onSave(bool checked);
    void onLoadFile(QString idx, QString filename, QString savepath);
    void onUpdateTP(int idx, double value, double lostrate);
    void selectRowBySettingCurrentCell(int rowToSelect);

signals:
    void loadfile(QString idx, QString filename, QString savepath);
    void reportTP(int idx, double tp, double lostrate);
protected:
    void changeEvent(QEvent *e);
    void initRightMenu();
private slots:
    void showContextMenu(const QPoint &pos);
    void onInsert(bool checked);
    void onDelete(bool checked);
    void onCopy(bool checked);
    void onPaste(bool checked);
    void onClear(bool checked);
    void onProgress(int value);
    void onStarted();
    void onStoped();

private:
    static const QByteArray MAGIC_VALUE;
    static const qint32 VERSION;
    uint32_t m_version;
    Ui::DlgSimpleMicro *ui;
    SimpleWorker *m_sworker;
    QThread *m_thread;
    QMenu *m_rightmenu;
    QAction *m_insertAction;
    QAction *m_deleteAction;
    QAction *m_copyAction;
    QAction *m_pasteAction;
    QAction *m_clearAction;
    QClipboard *m_clipboard;
    QString m_oldpath;
};

#endif // DLGSIMPLEMICRO_H
