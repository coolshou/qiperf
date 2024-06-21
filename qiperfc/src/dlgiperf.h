#ifndef DLGIPERF_H
#define DLGIPERF_H

#include <QDialog>
#include <QStringList>
#include <QCloseEvent>
#include <QEvent>
#include <QMap>
#include <QString>

namespace Ui {
class DlgIperf;
}

// class of iperf args config dialog
class DlgIperf : public QDialog
{
    Q_OBJECT

public:
    explicit DlgIperf(QWidget *parent = nullptr);
    ~DlgIperf() override;
    QString getJsonCfg();
    void loadJsonCfg(QString jsoncfg);
    bool add(QString mgr);
    bool add(QString mgr, QString mdata); // mgr: manager ip, mdata: relative data
    void updateUI();

public slots:
    void ChangeVersion(const QString ver);
    void onAccepted();
    void onChkBidirStatech(int state);
    void onChkReverseStatech(int state);
    void onSelectMServer(QString text);
    void onSelectMClient(QString text);


protected:
    void changeEvent(QEvent *e) override;
//    void closeEvent(QCloseEvent *event) override;

private:
    Ui::DlgIperf *ui;
    QStringList mgrls; //manager ip address list
    QMap<QString, QStringList> m_ips; // manager ip, all support ip in the manager server
    bool b_ipv6;
};

#endif // DLGIPERF_H
