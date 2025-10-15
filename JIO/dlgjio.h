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
#include <QThread>

#include <qssh/sshconnection.h>
#include <qssh/sshremoteprocessrunner.h>

// #include "../src/map/dlgopenstreetmap.h"
#include "../src/map/dlggeoosm.h"
#include "../src/gps/iplocationprovider.h"
#include "dlgaip.h"
#include "dlgset.h"
#include "dlgoptimize.h"
#include "dlgbeamcmd.h"
#include "optimizeworker.h"
#include "hanwha.h"
#include "dlghanwha.h"
#include "cyntec.h"
#include "dlgcyntec.h"
#include "aip.h"
#include "../src/wsclient.h"

enum PointLabel { UNCLASSIFIED = -1, NOISE = -2 };

struct DBPoint {
    QPointF pos;
    int label = UNCLASSIFIED;
};


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
        IPAddr=8,
        MacAddr=9
    };
    Q_ENUM(GPScols)
    enum AZEIcols{
        Name=0,
        Distance=1,
        P1Azimuth=2,
        P2Azimuth=3,
        P1Elevation=4,
        P2Elevation=5,
        P2AzDiff=6,
        P2ElDiff=7,
        BeamDirID=8,
        P2BFTx1Att=9,
        P2BFTx2Att=10,
        P2Tx1Att=11,
        P2Tx2Att=12,
        P2BFRx1Att=13,
        P2CRx1Att=13,
        P2BFRx2Att=14,
        P2CRx2Att=14,
        P2CLnaAtt=15,
        P2Rx1Att=15,
        P2Rx2Att=16,
        P2RxLnaAtt=17
    };
    Q_ENUM(AZEIcols)
    enum AIPcols{
        Azimuth=0,
        Elevation=1,
        Azdiff=2,
        BeamDirectionID=3,
        BFTx1Att=4,
        BFTx2Att=5,
        Tx1Att=6,
        Tx2Att=7,
        BFRx1Att=8,
        CRx1Att=8, //Cyntec Rx1Att
        BFRx2Att=9,
        CRx2Att=9,  //Cyntec Rx2Att
        CLnaAtt=10, //Cyntec Lna
        Rx1Att=10,
        Rx2Att=11,
        RxLnaAtt=12
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
    QString getGpsInfo(QString refrow, QString target);
    QString getSensorInfo(QString refrow, QString target);
    void getAPInfo(QString refrow, QString target);
    AIP::ModuleType getModuleType(int row, int col);
    QJsonObject createInitData();
    void getBestBeamID(int idx, double azimuthDegree, AIP::ModuleType aip1type,
                       QList<QTableWidgetItem*> cm7rs, double maxDistance=0.0);
    int findClosestBeamID(double targetAz, double targetEl,
                          QString beamtype="Narrow", int beamfactor=1);
    void initHanwhaBeamCMD(QString c, QString antarraymode="8x8", QString cmName="");
    void initHanwhaBeamIdCMD(QString c, QString beamid, QString cmName="");
    void initHanwhaBeamTxAttCMD(QString c, QString bfTx1, QString bfTx2,
                          QString Tx1att, QString Tx2att, QString cmName="");
    void initHanwhaBeamRxAttCMD(QString c, QString bfRx1, QString bfRx2,
                          QString Rx1att, QString Rx2att, QString RxLan,
                          QString cmName="");
    void initCyntecBeamCMD(QString c, QString antarraymode="8x8", QString cmName="");
    void initCyntecBeamIdCMD(QString c, QString beamid, QString cmName="");
    void initCyntecBeamTxAttCMD(QString c, QString Tx1att, QString Tx2att, QString cmName="");
    void initCyntecBeamRxAttCMD(QString c, QString Rx1att, QString Rx2att,
                                QString Rx1iip3, QString Rx2iip3, QString cmName="");

    void initResultHeader(AIP::ModuleType aip1type);

public slots:
    void onRequestResult(QString refrow, QString serveraddress, QString cmd, QString msg);
    void setTableWidgetBGColor(QTableWidget *tw, int row, int col, QColor color);
signals:
    void TileAvailable(bool ok);
    void closeAll();
    void highlightItm(QString label);
    void deleteItm(QString label);
    void requestExec(QString targetIP, QString idx, QString sCmd);
    void startOptimiz();//
    void stopOptimiz();//
    void sigAddIperf(QString cfg);
    void sigClearIperf();
    void addBeamIDCmd(QString cmd);
    void addCMBeamIDCmd(QString name, QString cmd);
    void clearBeamIDCmd();
    void clearCMBeamIDCmd();

protected:
    void changeEvent(QEvent *e) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onAddIperf(QString cfg);
    void doRequestExec(QString targetIP, QString idx, QString sCmd);
    void onStartOptimiz();
    void onStopOptimiz();
    void initHanwha();
    void showHanwha(bool checked);
    void initCyntec();
    void showCyntec(bool checked);
    void initCmds();
    void initTableWidget();
    void initAction();
    void onInsert(bool checked);
    void onDelete(bool checked);
    void onGetGPS(bool checked);
    void onGetSensor(bool checked);
    void onAddRow(QString name, double latitude, double longitude,
                  double altitude, double heading, double pitch,
                  QJsonObject aip1=QJsonObject(), QJsonObject aip2=QJsonObject(), QString ipaddr="");
    void onClear(bool checked);
    void onLoadCliecked(bool checked);
    void onSaveCliecked(bool checked);
    void onCalcClicked(bool checked);
    void onSet(bool checked);
    void onInquireClicked(bool checked);
    void onOptimizeClicked(bool checked);
    void onInquireTimerTimeout();
    void onDisconnected(QString from);
    void onConnected(QString from);
    // void onShowMap(bool checked);
    void onShowGeo(bool checked);
    void onShow3D(bool checked);
    void onToDMS(bool checked);
    void onToDegree(bool checked);
    void onCMBeamDirIDInit(bool checked);
    void onAttInit(bool checked=false);
    void onBeamDirIDCmd(bool checked);
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
    //opt
    void onOptimizeStarted();
    void onOptimizeStoped(int error);
    void onOptimizeWorkerDebug(QString msg);
private:
    QIcon iconForState(const QString &state);
    void updateStats(QString target, QString state);
    void setStateIcon(int row, int column, QString state);
    int getNearestBeamDirectionID(QString name, AIP::ModuleType aiptype, double diffHead, double diffPitch);
    double getAz(AIP::ModuleType aiptype, int BeamID);
    double averageBearing(const QList<double>& bearings);
    void getSelfIpLocation();
    void onLoad(QString filename);
    bool onSave(QString filename);
    void debug(QString msg, int lv=3);
    void loadcfg();
    void savecfg();
    QVector<QPointF> polarToXY(const QVector<double>& anglesDeg, const QVector<double>& distances);
    QVector<int> kMeansCluster(const QVector<QPointF>& points, int k = 2, int maxIter = 100);
    bool isAzimuthClose(double a1, double a2, double thresholdDeg = 3.5);
    // double bearing(double lat1, double lon1, double lat2, double lon2);
    // DBSCAN
    double euclideanDistance(const QPointF& a, const QPointF& b);
    QVector<int> regionQuery(const QVector<DBPoint>& points, int index, double eps);
    bool expandCluster(QVector<DBPoint>& points, int index, int clusterId, double eps, int minPts);
    QVector<int> dbscan(const QVector<QPointF>& inputPoints, double eps, int minPts);
    Ui::DlgJIO *ui;
    QSettings *m_cfg;
    QMenu *m_contextMenu;
    QAction *m_insertAction;
    QAction *m_deleteAction;
    QAction *m_GPSAction;
    QAction *m_SensorAction;
    QAction *m_clearAction;
    // DlgOpenStreetMap *m_dlgOSM;
    DlgGeoOSM *m_dlgGeo;
    DlgAIP *m_dlgaip;
    DlgOptimize *mDlgOptimize;
    DlgBeamCmd *mDlgBeamCmd;
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
    Hanwha *mHanwha;
    DlgHanwha *mDlgHanwha;
    Cyntec *mCyntec;
    DlgCyntec *mDlgCyntec;
    //cmds
    QJsonObject jiocmdObj;
    QMap<QString, WSClient *> mWScs;
    OptimizeWorker *mOptWorker;
    QThread *mOptThread;
    QStringList mHeaderResult;
    QStringList mHeaderAIP;
    QStringList mHeaderHanwha;
    QStringList mHeaderCyntec;
};

#endif // DLGJIO_H
