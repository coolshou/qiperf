#ifndef DLGJIO_H
#define DLGJIO_H

#include <QDialog>
#include <QPoint>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QNetworkReply>
#include <QJsonObject>
#include <QAbstractButton>
#include <QTimer>

#include <qssh/sshconnection.h>
#include <qssh/sshremoteprocessrunner.h>

// #include "../src/map/dlgopenstreetmap.h"
#include "../src/map/dlggeoosm.h"
#include "../src/gps/iplocationprovider.h"
#include "dlgaip.h"
#include "dlgset.h"

namespace Ui {
class DlgJIO;
}

class DlgJIO : public QDialog
{
    Q_OBJECT

public:
    enum GPScols{
        PositionName=0,
        Latitude=1,
        Longitude=2,
        Altitude=3,
        Heading=4,
        Pitch=5,
        AIP1=6,
        AIP2=7,
        IPAddr=8
    };
    Q_ENUM(GPScols)
    enum AZEIcols{
        Name=0,
        Distance=1,
        Azimuth1=2,
        Azimuth2=3,
        Elevation1=4,
        Elevation2=5
    };
    Q_ENUM(AZEIcols)
    enum AIPcols{
        Azimuth=0,
        Elevation=1,
        Azdiff=2,
        BeamDirectionID=3
    };
    Q_ENUM(AIPcols)
    enum State {
        Inactive, TestingSuccess, TestingFailure, TestingCrash, TestingTerminal, TestingIoDevice,
        TestingProcessChannels
    };
    Q_ENUM(State)
    explicit DlgJIO(QSettings *cfg, QWidget *parent = nullptr);
    ~DlgJIO() override;
    void isTileAvailable();
    QString getTile();
    void setShowLine(bool show);
    void clearData();
    QString getStMotion(QString target);
    QString getGpsInfo(QString target);
signals:
    void TileAvailable(bool ok);
    void closeAll();
    void highlightItm(QString label);

protected:
    void changeEvent(QEvent *e) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void initAction();
    void onInsert(bool checked);
    void onDelete(bool checked);
    void onAddRow(QString name, double latitude, double longitude,
                  double altitude, double heading, double pitch,
                  QJsonObject aip1=QJsonObject(), QJsonObject aip2=QJsonObject(), QString ipaddr="");
    void onClear(bool checked);
    void onLoadCliecked(bool checked);
    void onSaveCliecked(bool checked);
    void onCalcCliecked(bool checked);
    void onSet(bool checked);
    void onInquireClicked(bool checked);
    void onInquireTimerTimeout();
    // void onShowMap(bool checked);
    void onShowGeo(bool checked);
    void onShow3D(bool checked);
    void onToDMS(bool checked);
    void onToDegree(bool checked);
    void showContextMenu(const QPoint &pos);
    void onDeviceCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void onLoadFinished(bool ok);
    void onAddPosition(QString label, double lat, double lon);
    void onTileAvailable(bool ok);
    void onCheckTileFinished();
    void onCheckTileErrorOccurred(QNetworkReply::NetworkError errorcode);
    void handleButtonClicked(int row, int col);
    void onAcceptedAIP();
    // void onUpdateData(int row, int col, QString data);
    void onUpdateData(int row, int col, QJsonObject data);
    void onUpdateModelType(int row, int col, QString smodel);
    void onUpdateModelType(int row, int col, int model);
    void onUpdateSetting(QString sshusername, QString sshpassword,
                         QString webusername, QString webpassword,
                         DlgSet::ControlBy ctl);
    void onLocationReady(const IpLocation& location);
    // SSH
    void handleSSHConnectionError();
    void handleSSHProcessStarted();
    void handleSSHProcessStdout();
    void handleSSHProcessStderr();
    void handleSSHProcessClosed(int exitStatus);

private:
    double averageBearing(const QList<double>& bearings);
    void getSelfIpLocation();
    void onLoad(QString filename);
    bool onSave(QString filename);
    void debug(QString msg, int lv=3);
    void loadcfg();
    void savecfg();
    // double bearing(double lat1, double lon1, double lat2, double lon2);
    Ui::DlgJIO *ui;
    QSettings *m_cfg;
    QMenu *m_contextMenu;
    QAction *m_insertAction;
    QAction *m_deleteAction;
    QAction *m_clearAction;
    // DlgOpenStreetMap *m_dlgOSM;
    DlgGeoOSM *m_dlgGeo;
    DlgAIP *m_dlgaip;
    QNetworkReply *reply = nullptr;
    bool showline=false;
    QString m_oldsavepath;
    int m_debuglv;
    QTimer * m_InquireTimer;
    DlgSet *m_dlgset;
    QString mSshUsername;
    QString mSshPassword;
    QString mWebusername;
    QString mWebpassword;
    DlgSet::ControlBy  mControlBy; //1 : ssh, 2: qiperfd
    IpLocationProvider* provider;
    IpLocation mIpLocation;
    QSsh::SshConnectionParameters m_sshParams;
    QSsh::SshRemoteProcessRunner *mSSHRemoteRunner;
    QByteArray m_remoteStdout;
    QByteArray m_remoteStderr;
    State m_state;
    bool m_started;
    QScopedPointer<QTextStream> m_textStream;
};

#endif // DLGJIO_H
