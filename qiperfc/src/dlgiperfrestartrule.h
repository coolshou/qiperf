#ifndef DLGIPERFRESTARTRULE_H
#define DLGIPERFRESTARTRULE_H

#include <QDialog>
#include <QJsonObject>
#include <QMenu>
#include <QAction>
#include <QTableWidgetItem>
#include <QSettings>

namespace Ui {
class DlgIperfRestartRule;
}

class DlgIperfRestartRule : public QDialog
{
    Q_OBJECT
public:
    enum cols{
        Enable=0,
        Keyword,
        Count,
        Comment
    };
    Q_ENUM(cols)

    explicit DlgIperfRestartRule(QSettings *cfg, QWidget *parent = nullptr);
    ~DlgIperfRestartRule();
    QJsonObject getJsonCfgObj();
    void loadcfg(QSettings *cfg);
    void setJsonRules(QJsonObject rule);
protected:
    void changeEvent(QEvent *e)override;
private slots:
    void onAddRule(bool checked);
    void onDelRule(bool checked);
    void handleItemChanged(QTableWidgetItem *item);
    void showContextMenu(const QPoint &pos);
    void onSaveDefault(bool checked);
private:
    Ui::DlgIperfRestartRule *ui;
    QSettings *m_cfg;
    QMenu *m_rmenu;
    QAction *m_AddAction;
    QAction *m_DelAction;
    void initMenu();
    void addRowData(bool enable, const QString &detect, int count, const QString &comment);
    void jsonToRowData(QJsonObject obj);
};

#endif // DLGIPERFRESTARTRULE_H
