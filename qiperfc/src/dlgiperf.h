#ifndef DLGIPERF_H
#define DLGIPERF_H

#include <QDialog>
#include <QStringList>
#include <QCloseEvent>
#include <QEvent>
#include <QMap>
#include <QString>
#include <QtGlobal>
#include <QSettings>

#include "tpmgr.h"
#include "dlgiperfrestartrule.h"

namespace Ui {
class DlgIperf;
}

// class of iperf args config dialog
class DlgIperf : public QDialog
{
    Q_OBJECT

public:
    explicit DlgIperf(TPMgr *tpmgr, QSettings *cfg, QWidget *parent = nullptr);
    ~DlgIperf() override;
    QString getJsonCfg();
    void loadJsonCfg(QString jsoncfg);
    bool add(QString mgr);
    bool add(QString mgr, QString mdata); // mgr: manager ip, mdata: relative data
    void updateUI();
    void setExcIdx(QModelIndex excIdx);

public slots:
    void ChangeVersion(const QString ver);
    void onAccepted();
#if QT_VERSION < QT_VERSION_CHECK(6,7,0)  // < 6.7
    void onChkBidirStatech(int state);
    void onChkReverseStatech(int state);
    void onTimeStampstateChanged(int state);
    void onChkRestartOnError(int state);
#else
    void onChkBidirStatech(Qt::CheckState state);
    void onChkReverseStatech(Qt::CheckState state);
    void onTimeStampstateChanged(Qt::CheckState state);
    void onChkRestartOnError(Qt::CheckState state);
#endif
    void onSelectMServer(QString text);
    void onSelectMClient(QString text);
    void onMSSvalueChanged(int value);
    void showManagement(bool show);
    void onDurationValueChanged(int value);
    void onFmtreportChanged(QString text);
    void onShowIperfRestartRule(bool checked);

protected:
    void changeEvent(QEvent *e) override;
//    void closeEvent(QCloseEvent *event) override;

private:
    bool isRequireConfigMeet();
    Ui::DlgIperf *ui;
    TPMgr *m_tpmgr;
    QStringList mgrls; //manager ip address list
    QMap<QString, QStringList> m_ips; // key: manager ip, value: all support ip in the manager server
    bool b_ipv6;
    QModelIndex m_excIdx;
    int old_mss; // store old mss value
    bool m_enabled=true;
    DlgIperfRestartRule *m_dlgrule;
};

#endif // DLGIPERF_H
