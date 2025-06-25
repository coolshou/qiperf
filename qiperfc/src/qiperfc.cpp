#include "qiperfc.h"
#include "ui_qiperfc.h"
#include "../src/comm.h"
#include "../src/myfunc.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <QStringLiteral>
#include <QUrl>
#include <QFileDialog>
#include <QStandardPaths>
#include <QSaveFile>
#include <QThread>
#include <QCoreApplication>
#include <QEventLoop>
#include <QTreeView>
#include <QToolTip>
#include <QAction>
#include <QCursor>

#include <QVBoxLayout>

#include "endpointact.h"
#include "tp.h"
#include "versions.h"
#include "views/viewtype.h"

#include <QDebug>

QIperfC::QIperfC(QString logpath, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    settingfilepath =  QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    // /home/jimmy/.local/share/alphanetworks/qiperfconsole
    QDir d{settingfilepath};
    if (!d.exists()){
        if(!d.mkpath(settingfilepath)){
            qDebug() << "ERROR: mkdir " + settingfilepath + " Fail";
        }
    }
    QString settingfilename = settingfilepath + QDir::separator() + QIPERFC_NAME + ".ini";
    qInfo() << "settingfilename:" << settingfilename;
    m_TestStartTime = QDateTime();
    m_settings=new QSettings(settingfilename, QSettings::IniFormat);
    m_clipboard = QApplication::clipboard();
    ui->setupUi(this);
    loadPlugins();
    loadSettings();
    m_smicroIdx = -1;
    m_logpath = logpath + "data";
    QDir logdir(m_logpath);
    if (!logdir.exists()){
        // qDebug() << "create path: " << m_logpath;
        logdir.mkpath(".");
    }
    // connect(this, &QIperfC::testStarted, this, &QIperfC::onTestStarted);
    connect(this, &QIperfC::testStoped, this, &QIperfC::onTestStoped);
    connect(this, &QIperfC::doNtpSync, this, &QIperfC::onDoNtpSync);
    ui->actionPaste->setShortcutContext(Qt::WidgetShortcut);
    m_throughputview = new ThroughputView(ui->actionCopy, ui->actionPaste,
                                          ui->actionDelete, ui->actionCopyText,
                                          m_TPGroup, m_TPUnit);
    connect(m_throughputview, &ThroughputView::updateActions, this, &QIperfC::onUpdateActions);
    connect(m_throughputview, &ThroughputView::updateActionsSave, this, &QIperfC::onUpdateActionsSave);
    connect(m_throughputview, &ThroughputView::updateActionsEdit, this, &QIperfC::onUpdateActionsEdit);
    connect(this, &QIperfC::setEndTime, m_throughputview, &ThroughputView::setXRangeUpper);
    connect(this, &QIperfC::updateInterval, m_throughputview, &ThroughputView::setInterval);

    m_views = new ViewManager(&settingfilepath, m_throughputview, this);
    m_dlgtest = new DlgTest();
    m_dlgserial = new DlgSerial(); //for serial port config
    m_dlgssh = new DlgSSH();
    m_dlgoption = new dlgOption(m_settings);
    connect(m_dlgoption, &dlgOption::widthChanged, this, &QIperfC::onWidthChanged);
    connect(m_dlgoption, &dlgOption::heigthChanged, this, &QIperfC::onHeigthChanged);
    connect(m_dlgoption, &dlgOption::showGroup, this, &QIperfC::onShowGroup);
    connect(m_dlgoption, &dlgOption::IgnoreWrongInterval, this, &QIperfC::onIgnoreWrongInterval);
    connect(m_dlgoption, &dlgOption::updateTPUnit, this, &QIperfC::onUpdateTPUnit);
    connect(m_dlgoption, &dlgOption::updateTPUnit, m_throughputview, &ThroughputView::onUpdateTPUnit);
    connect(m_dlgoption, &dlgOption::updateOpenStreetMapTile, this, &QIperfC::onUpdateOpenStreetMapTile);
    initStatusbar();

    //UI actions
    initActions();
    initToolbar();
    updateRunStatus(false);
    // initPingChart();
    // connect(this, &QIperfC::errorStop, this, &QIperfC::onErrorStop);

    iTimeout = 10*1000;//10sec
    //
    m_qipconfig = new QIPConfig(logdir.absolutePath(), m_IgnoreWrongInterval);
    connect(m_qipconfig, &QIPConfig::updateDataPath, this, &QIperfC::onUpdateDataPath);
    connect(m_qipconfig, &QIPConfig::updateTPCfg, m_throughputview, &ThroughputView::onUpdateTPCfg);
    connect(m_qipconfig, &QIPConfig::updateStartDateTime, this, &QIperfC::setStartTime);
    connect(m_qipconfig, &QIPConfig::updateStartDateTime, m_throughputview, &ThroughputView::setStartTime);
    // connect(m_qipconfig, &QIPConfig::updateTPDatas, m_throughputview, &ThroughputView::onUpdateTPDatas);
    connect(m_qipconfig, &QIPConfig::updateTPAvg, m_throughputview, &ThroughputView::onAddTPdata); // this only set last avg, which may cause min/max value wrong!!
    connect(m_qipconfig, &QIPConfig::updateTPDatas, m_throughputview, &ThroughputView::onUpdateTPDatas);
    connect(m_qipconfig, &QIPConfig::progress, this, &QIperfC::onProgress);
    connect(m_throughputview, &ThroughputView::deleteFiles, m_qipconfig, &QIPConfig::onDeleteFiles);
    connect(m_throughputview, &ThroughputView::showGroup, this, &QIperfC::setShowGroup);

    QString proxyhost="";
    quint16 proxyport=0;
    if (m_qipconfig->detectSystemProxy(proxyhost, proxyport)){
        onError("Detect system have proxy setting (proxy="+ proxyhost +":"+QString::number(proxyport)+"), which may cause qiperfd control problem!!");
    }
    m_endpointmgr = new EndPointMgr(this);
    m_frm_qiperfds = new FormQIperfds();
    m_frm_qiperfds->setModel(m_endpointmgr);
    connect(m_frm_qiperfds, &FormQIperfds::clearNtpStatus, this, &QIperfC::onClearNtpStatus);
    //
    m_receiver = new UdpReceiver(QIPERFD_BPORT, this);
    connect(m_receiver, &UdpReceiver::notice, this, &QIperfC::onNotice);
    connect(m_receiver, &UdpReceiver::error, this, &QIperfC::onError);
    m_receiver->start();

    // control local qiperfd?
//    pclient = new PipeClient(QIPERFD_NAME);
//    connect(pclient, SIGNAL(newMessage(QString)), this, SLOT(onNewMessage(QString)));
//    pclient->SetAppHandle(qApp);

    m_dlgrecord = new DlgRecord();
    m_fileserver = new FileServer(QIPERF_FILEPORT);
    connect(m_fileserver, &FileServer::error, this, &QIperfC::onFileServerError);

#if (TEST_ICMP==1)
    dp = new DlgPing(this);
#endif
    m_dlgshowlog=new DlgShowLog(logpath+QIPERFC_NAME+".log", this);
    connect(this, &QIperfC::closeAll, m_dlgshowlog, &DlgShowLog::close);
    connect(this, &QIperfC::closeAll, m_dlgrecord, &DlgRecord::close);
    connect(this, &QIperfC::closeAll, m_frm_qiperfds, &FormQIperfds::close);
    connect(this, &QIperfC::closeAll, m_dlgserial, &DlgSerial::close);
    connect(this, &QIperfC::closeAll, m_dlgssh, &DlgSerial::close);

    connect(this, &QIperfC::closeAll, m_dlgtest, &DlgTest::close);
    connect(this, &QIperfC::closeAll, m_views, &ViewManager::close);


#if (DEBUG_EXPORT_HTML==1)
    m_debugdlg = new QDialog(this);
    m_debugdlg->setModal(false);
    m_debugdlg->resize(1280,1024);
#endif

    m_serialviews = new QMap<QString, SerialData>();
    m_sshviews = new QMap<QString, SSHData>();
}

QIperfC::~QIperfC()
{
//    qDebug() << "~QIperfC";
#if (DEBUG_EXPORT_HTML==1)
    delete m_debugdlg;
#endif
    if (m_dlgrecord){
        delete m_dlgrecord;
    }
    delete ui;
}

bool QIperfC::load(QString filename)
{    //load test config file
    if (m_throughputview->rootChildCount()>0) {
        if (m_smicroIdx>=0){
            onClear(false);
        }else{
            QMessageBox msgBox;
            msgBox.setText("Clear data before load config");
            msgBox.setInformativeText("Do you want to save your changes?");
            msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Save);
            int ret = msgBox.exec();
            switch (ret) {
            case QMessageBox::Save:
                // Save was clicked
                onSave();
                break;
            case QMessageBox::Discard:
                // Don't Save was clicked
                on_Clear(false);
                break;
            case QMessageBox::Cancel:
                // Cancel was clicked
                return false;
                //break;
            default:
                // should never be reached
                break;
            }
        }
    }
    onNew();
    if (m_qipconfig->loadFromFile(filename)){
        ui->actionSave->setEnabled(true);
        return true;
    }else{
        qDebug() << "load file " << filename << " Fail!!";
    }
    return false;
}

bool QIperfC::save(QString filename)
{
    //prepare throughput config data
    if (m_throughputview->rootChildCount()>0) {
        QByteArray b = m_throughputview->savedata();
        QStringList pcs = m_throughputview->getPCs();
        QString env= m_endpointmgr->getPCsInfo(pcs);
//        qDebug() << "env: " << env;
        QString starttime="";
        if (m_TestStartTime.isValid()){
            starttime = m_TestStartTime.toString(DATETIME_NOW_FORMAT);
            QString tmp = m_logpath + QDir::separator() + starttime;
            QDir d(tmp);
            QStringList filelist;
            foreach(auto s, d.entryList(QDir::Files)){
                filelist.append(tmp+ QDir::separator()+s);
            }
            m_qipconfig->setTPCfg(b, env, starttime, filelist);
        }else{
            m_qipconfig->setTPCfg(b, env);
        }
        m_qipconfig->saveToFile(filename);
        return true;
    }else {
        qDebug() << "NO throughput config to save";
        return false;
    }
}

QString QIperfC::getNowString()
{
    return QDateTime::currentDateTime().toString(DATETIME_NOW_FORMAT);
}

void QIperfC::onNewMessage(const QString msg)
{
    m_dlgtest->append(msg);
}

void QIperfC::onNew()
{
    ui->actionSave->setEnabled(false);
    on_Clear();
    if (m_throughputview->rootChildCount()>0) {
        //this will clear all item include root!!
        m_throughputview->reset();
    }/*else{
        qDebug() << "onNew rootChildCount No child";
    }*/
}

void QIperfC::onOpen()
{
//    onNew();
    QString path;
    if (!m_oldsavepath.isNull()){
        path = m_oldsavepath;
    }else {
        path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }
    QString fileName = QFileDialog::getOpenFileName(this,
             tr("Open QIperf file"), path , tr(QIPERF_EXT_FILTER));
    if (!fileName.isEmpty()){
        QFileInfo fi(fileName);
        QString ext = fi.suffix();
        if (ext.compare(QIPERF_EXT)!=0){
            qDebug() << "Not support file ext format: " << ext << " Expect:" << QIPERF_EXT;
            return;
        }
        // doClear();
        // onNew();
        if (load(fileName)){
            m_oldsavepath = fi.path();
        }
    }
}

void QIperfC::onSave()
{
    QString path;
    if (!m_oldsavepath.isNull()){
        path = m_oldsavepath;
    }else {
        path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }
    QString fileName = QFileDialog::getSaveFileName(this,
             tr("Save QIperf "), path, tr(QIPERF_EXT_FILTER));
    QFileInfo fi(fileName);
    QString ext = fi.suffix();
    if (ext.compare(QIPERF_EXT)!=0){
        fileName = fi.path()+ "/" + fi.baseName() + "."+ QIPERF_EXT;
    }
//    qInfo() << "save file: " << fileName ;
    if (save(fileName)){
        m_oldsavepath = fi.path();
    }

}

void QIperfC::onImportIperf3Log()
{
    qDebug() << "TODO:  Import Iperf3 Log file to throughput chart";
    QString path;
    if (!m_oldsavepath.isNull()){
        path = m_oldsavepath;
    }else {
        path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }
    QString fileName = QFileDialog::getOpenFileName(this,
             tr("Open Iperf3 log file"), path , tr(ALL_EXT_FILTER));
    if (!fileName.isEmpty()){
        if(!m_qipconfig->importIperf3Log(fileName)){
            qDebug() << "Import file: " << fileName << " Fail!!";
        }else{
            QFileInfo fileInfo(fileName);
            QString s= fileInfo.absoluteFilePath();
            qDebug() <<"s:" << s;
            m_oldsavepath = s;
        }
    }
}

void QIperfC::onImportIperf2Log()
{
    QString path;
    if (!m_oldsavepath.isEmpty()){
        path = m_oldsavepath;
    }else {
        path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    }
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Iperf2 log file"), path , tr(ALL_EXT_FILTER));
    // QString fileName = "/home/jimmy/work/qiperf/src/iperf2-TCP.txt"; //TCP tmp
    //QString fileName = "/home/jimmy/work/qiperf/src/iperf2-UDP.txt"; //UDP tmp
    if (!fileName.isEmpty()){
        if(!m_qipconfig->importIperf2Log(fileName)){
            QString err("Import file: " + fileName + " Fail!!");
            onError(err);
        }else{
            QFileInfo fileInfo(fileName);
            QString s= fileInfo.absoluteFilePath();
            qDebug() <<"s:" << s;
            m_oldsavepath = s;
        }
    }

}

bool QIperfC::on_Clear(bool showNotice)
{
    // this will clean iperf test pair config
    return onClear(showNotice);
}

void QIperfC::onStart(bool showNotice)
{
    if (!onClear(showNotice)){
        return;
    }
    if (m_throughputview->rootChildCount()>0) {
        // list of throughput test pair
        QList<TP *> tps = m_throughputview->getChilds();
        qDebug() << "onStart tps:" << tps;
        m_tpworker = new TpWorker(m_logpath, tps);
        connect(m_tpworker, &TpWorker::testStarted, this, &QIperfC::onTestStarted);
        connect(m_tpworker, &TpWorker::testStoped,this,  &QIperfC::onTestStoped);
        connect(m_tpworker, &TpWorker::updateDatapath, this, &QIperfC::onUpdateDataPath);
        connect(m_tpworker, &TpWorker::updateStarttime, this, &QIperfC::onUpdateStarttime);
        connect(m_tpworker, &TpWorker::updateRunStatus, this, &QIperfC::onUpdateRunStatus);
        connect(m_tpworker, &TpWorker::updateStatus, this,  &QIperfC::onUpdateStatus);
        connect(m_tpworker, &TpWorker::errorStop, this, &QIperfC::onErrorStop);
        connect(m_tpworker, &TpWorker::updateComment, m_throughputview, &ThroughputView::addComment);
        connect(m_tpworker, &TpWorker::iperfTPdata, m_throughputview, &ThroughputView::onIperfTPdata);
        connect(m_tpworker, &TpWorker::setEndTime, m_throughputview, &ThroughputView::setXRangeUpper);
        connect(m_tpworker, &TpWorker::debuginfo, this, &QIperfC::onDebuginfo);
        connect(this, &QIperfC::setTPStop, m_tpworker, &TpWorker::onSetStop);

        m_tpthread = new QThread();
        connect(m_tpthread, &QThread::started, m_tpworker, &TpWorker::work);
        connect(m_tpthread, &QThread::finished, m_tpthread, &QThread::deleteLater);
        connect(m_tpthread, &QThread::finished, m_tpworker, &TpWorker::deleteLater);
        m_tpworker->moveToThread(m_tpthread);
        m_tpthread->start();

    } else {
        QMessageBox::information(this,"NOTICE", "Plase add iperf test pair first!", QMessageBox::Ok);
        emit testStoped(-1);
    }
}

void QIperfC::onStop(){
    emit setTPStop();
}

bool QIperfC::onClear(bool showNotice){
    if (m_TestStartTime.isValid() && showNotice){
        int ret = QMessageBox::information(this, "NOTICE", "Previous test record will be clear, Continious?", QMessageBox::Ok|QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel){
            // test cancel
            return false;
        }
    }
    doClear();
    return true;
}

void QIperfC::onAddPing()
{
    QString strJson;
#if (TEST_ICMP==1)
    if (dp->exec()== QDialog::Accepted){
        strJson = dp->getJsonstr();
        qDebug() << "strJson:" << strJson;
        //TODO: add to ping treeview/chart
        ui->actionSave->setEnabled(true);
    }
#endif
}

void QIperfC::onWlanSTA()
{
    // TODO: add wlan sta monitor
}

void QIperfC::onError(QString msg)
{
    QMessageBox::warning(this, "ERROR", msg);
}

void QIperfC::onFileServerError(QString msg)
{
    QMessageBox::warning(this, "ERROR", msg);
}

void QIperfC::onShowLog()
{
    if (!m_datapath.isEmpty()){
        QDir d(m_datapath);
        if (d.exists()){
            m_dlgrecord->setRootPath(m_datapath);
            m_dlgrecord->show();
            m_dlgrecord->raise();
            m_dlgrecord->activateWindow();

        }else{
            QMessageBox::information(this, "ERROR", "No test record folder: " + m_datapath);
        }
    }else{
        QMessageBox::information(this, "ERROR", "No test record");
    }
}

void QIperfC::onConfig()
{
    m_dlgoption->setWaitServerReady(m_WaitServerReady);
    int rc = m_dlgoption->exec();
    if (rc == QDialog::Accepted){
        //update setting
        m_WaitServerReady = m_dlgoption->getWaitServerReady();
    }
}

void QIperfC::onSimpleMicro()
{
    smicro = new DlgSimpleMicro(this);
    connect(smicro, &DlgSimpleMicro::loadfile, this, &QIperfC::onAutoLoadFile);
    connect(this, &QIperfC::reportTP, smicro, &DlgSimpleMicro::onUpdateTP);
    smicro->show();
}

void QIperfC::onAbout()
{
    QMessageBox::about(this, "About", QString(QIPERFC_NAME)+
                       "\n v"+QString(QIPERFC_VERSION)+
                       "\n git:" + GITBRANCH+"-"+GITVER+
                       "\n Auther: Jimmy Yeh"
                       "\nURL: https://github.com/coolshou/qiperf");
}

void QIperfC::onShowDebugLog()
{
    m_dlgshowlog->show();
    // m_dlgshowlog->open();
    m_dlgshowlog->raise();
    m_dlgshowlog->activateWindow();
}

void QIperfC::onErrorStop(int err, QString msg)
{
    bErrorStop = err;
    m_ErrorMSG = msg;
    qDebug() << "onErrorStop: (" <<bErrorStop <<") " << m_ErrorMSG;
    emit updateStarttime(QDateTime());
    emit updateStatus(msg);
    updateRunStatus(false);
    emit testStoped(bErrorStop);
}

void QIperfC::onDebuginfo(QString msg)
{
    qDebug() << msg;
}

void QIperfC::onNotice(QString send_addr, QString msg)
{
    //receive qiperfd notices
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8(), &error);
    if (error.error == QJsonParseError::NoError){
    // check validity of the document
//    if(!doc.isNull()) {
        QJsonObject obj = doc.object();
        int act = obj["ACT"].toInt();
        switch (act){
            case EndPointAct::Add:
                // qDebug() << "qiperfd add:" << send_addr << "\nmsg:" << msg;
                if (m_endpointmgr->add(send_addr, msg)){
                    m_throughputview->addEndpoint(send_addr, msg);
                    emit updateEndpointNum(m_endpointmgr->getTotalEndpoints());
                }
                if (!m_ntps.contains(send_addr)){
                    int itry=0;
                    if (m_ntpfail.contains(send_addr)){
                        itry=m_ntpfail[send_addr];
                        if (itry>5){
                            return;
                        }
                    }
                    qDebug() << "ask NTP sync:" << send_addr << "(try:"<<QString::number(itry)<<")";
                    emit doNtpSync(send_addr);
                }
                break;
            case EndPointAct::Update:
                qDebug() << "TODO qiperfd Update: from(" << send_addr << ") " << msg;

                break;
            case EndPointAct::Del:
                qDebug() << "TODO qiperfd Del: from(" << send_addr << ") " << msg;
                break;
            case EndPointAct::Disable:
                m_endpointmgr->disable(send_addr);
                break;
            default:
                qDebug() << "TODO on_notice default action: from(" << send_addr << ") " << msg;
        }
    } else {
        qDebug() << "TODO on_notice invalid message: from(" << send_addr << ") " << msg;
        qDebug() << "ERROR: " << error.errorString();
    }
}

void QIperfC::onQuit()
{
    qInfo() << "onQuit";
    // TODO: do any thing before quit!
    qApp->quit();
}

void QIperfC::notificationReceived(const QString key, const QVariant value)
{
    qDebug() << "RPC Received notification:"
                     << "Key:" << key
                     << "Value:" << value;
}

void QIperfC::setStartTime(QDateTime startTime)
{
    // qDebug() << "QIperfC::setStartTime: " << startTime.toString(DATETIME_NOW_FORMAT);
    m_TestStartTime = startTime;
}

void QIperfC::onTestStarted()
{
    updateRunStatus(true);
}

void QIperfC::onTestStoped(int err)
{
    updateRunStatus(false);
    if (err){
        qDebug() << "onTestStoped: ERROR" << QString::number(err);
    }else{
        qDebug() << "onTestStoped: no error";
    }

    qDebug() << "Do we need to close m_fileserver?? m_fileserver sockets:" << QString::number(m_fileserver->getSockets());
    // if (m_fileserver->getSockets()>0){
    //     m_fileserver->close();
    // }
}

void QIperfC::onCopy()
{
    AbstractView* v = m_views->findActiveView();
    if(v){
        v->onCopy();
    }

}

void QIperfC::onPaste()
{
    AbstractView* v = m_views->findActiveView();
    if(v){
        v->onPaste();
    }
}

void QIperfC::onDelete()
{
    AbstractView* v = m_views->findActiveView();
    if(v){
        v->onDelete();
    }
}

void QIperfC::onCopyText()
{
    AbstractView* v = m_views->findActiveView();
    if(v){
        v->onCopyText();
    }
}

void QIperfC::closeEvent(QCloseEvent *event)
{
    //TODO: check config edit.
    Q_UNUSED(event)

    saveSettings();
    emit closeAll();
}

bool QIperfC::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == m_label_qiperfd && event->type() == QMouseEvent::MouseButtonPress) {
//        m_frm_qiperfds->setGeometry();
        int dx = m_frm_qiperfds->geometry().width()/2;
        int dy = m_frm_qiperfds->geometry().height()/2;
        QPoint p(this->geometry().center().x()-dx, this->geometry().center().y()-dy);
        m_frm_qiperfds->move(p);
        m_frm_qiperfds->show();
        m_frm_qiperfds->raise();
        m_frm_qiperfds->activateWindow();
    }
//    if((obj == ui->menubar || obj == ui->toolBar) &&
//            (event->type() == (Qt::Key_Control & QMouseEvent::MouseButtonPress))) {
//        qDebug() << "show menuTest";
//        ui->menuTest->setVisible(true);
//        ui->menuTest->setEnabled(true);

//    }

    return QObject::eventFilter(obj,event);
}

void QIperfC::loadPlugins()
{
    QDir pluginsDir(qApp->applicationDirPath());
        // Adjust path for deployment: usually 'plugins' or specific subdirectories
#ifdef Q_OS_WIN
    if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
        pluginsDir.cdUp();
    pluginsDir.cd("plugins"); // Example: expect plugins in a 'plugins' subdirectory
#elif defined(Q_OS_UNIX)
    if (pluginsDir.dirName().toLower() == "bin") // Common for Linux/macOS build structures
        pluginsDir.cdUp();
    pluginsDir.cd("plugins"); // Example: expect plugins in a 'plugins' subdirectory
#endif

    if (!pluginsDir.exists()) {
        qWarning() << "Plugins directory not found:" << pluginsDir.absolutePath();
        return;
    }

    qDebug() << "Searching for plugins folder in:" << pluginsDir.absolutePath();

    for (const QString &fileName : pluginsDir.entryList(QDir::Files)) {
        if (QLibrary::isLibrary(fileName)) { // Check if it's a valid library file
            QPluginLoader *loader = new QPluginLoader(pluginsDir.absoluteFilePath(fileName));
            QObject *plugin = loader->instance();

            if (plugin) {
                // Try to cast the loaded plugin to our interface
                PluginInterface *iPlugin = qobject_cast<PluginInterface *>(plugin);
                if (iPlugin) {
                    qDebug() << "Loaded plugin:" << iPlugin->pluginName();
                    plugins.append(iPlugin);
                    pluginLoaders.append(loader);

                    // Add plugin's menu to the main menu bar
                    QMenu* pluginMenu = iPlugin->createPluginMenu(this);
                    if (pluginMenu) {
                        // ui->menubar->addMenu(pluginMenu);
                        ui->menuDevice->addMenu(pluginMenu);
                    }

                    iPlugin->initialize(); // Call plugin's initialization method
                } else {
                    qWarning() << "Could not cast plugin" << fileName << "to PluginInterface.";
                    qWarning() << loader->errorString();
                    loader->unload(); // Unload if it's not our expected plugin type
                    delete loader;
                }
            } else {
                qWarning() << "Failed to load plugin:" << fileName;
                qWarning() << loader->errorString();
                delete loader;
            }
        }
    }
}

void QIperfC::unloadPlugins()
{
    // Unload plugins
    for (QPluginLoader* loader : qAsConst(pluginLoaders)) {
        if (loader->isLoaded()) {
            loader->unload();
        }
        delete loader;
    }
}
void QIperfC::updateRunStatus(bool bStart)
{
    //set button status
    ui->actionAddIperf->setEnabled(!bStart);
    onUpdateActionsEdit(!bStart, !bStart, !bStart, !bStart);
    onUpdateActions(!bStart, bStart, !bStart);
    bStartTest = bStart;
}


// void QIperfC::initPingChart()
// {
//     if (!m_testping){
// //        ui->tab_ping->setVisible(false);
//         ui->tabwidget->setTabVisible(1, false);
//     }
//     m_pingmgr = new PingMgr();
//     // ping chart
//     m_pingplot = new PingPlot(ui->widget_ping);
//     ui->hl_ping->addWidget(m_pingplot);
// }

void QIperfC::resetError()
{
    bErrorStop = 0;
    m_ErrorMSG = "";
}

void QIperfC::saveSettings()
{
    m_settings->beginGroup("MainWindow");
    m_settings->setValue("geometry", saveGeometry());
    m_settings->setValue("windowState", saveState());
    m_settings->setValue("oldsavepath", m_oldsavepath);
    m_settings->endGroup();
    m_settings->beginGroup("Iperf");
    m_settings->setValue("WaitServerReady", m_WaitServerReady);
    m_settings->setValue("TPExportWidth", m_TPExportWidth);
    m_settings->setValue("TPExportHeigth", m_TPExportHeigth);
    m_settings->setValue("TPGroup", m_TPGroup);
    m_settings->setValue("TPUnit", m_TPUnit);
    m_settings->setValue("IgnoreWrongInterval", m_IgnoreWrongInterval);
    m_settings->endGroup();
    m_settings->beginGroup("gps");
    m_settings->setValue("OpenStreetMapTile", m_OpenStreetMapTile);
    m_settings->endGroup();
    m_settings->sync();  // forces to write the settings to storage
}

void QIperfC::loadSettings()
{
    m_settings->beginGroup("MainWindow");
    // default to screen center
    QRect screen = QGuiApplication::primaryScreen()->geometry();
    int x = (screen.width()-rect().width())/2;
    int y = (screen.height()-rect().height())/2;
    QRect newrect = QRect(x, y, rect().width(), rect().height());
    move(x,y);
    restoreGeometry(m_settings->value("geometry", newrect).toByteArray());
    restoreState(m_settings->value("windowState").toByteArray());
    m_oldsavepath = m_settings->value("oldsavepath",
                                      QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).toString();
    m_settings->endGroup();

    m_settings->beginGroup("Iperf");
    m_WaitServerReady =m_settings->value("WaitServerReady", 10).toInt();
    m_TPExportWidth =m_settings->value("TPExportWidth", 1280).toInt();
    m_TPExportHeigth =m_settings->value("TPExportHeigth", 180).toInt();
    m_TPGroup = m_settings->value("TPGroup", false).toBool();
    m_TPUnit = m_settings->value("TPUnit", "Mbits/sec").toString();
    m_IgnoreWrongInterval = m_settings->value("IgnoreWrongInterval", false).toBool();
//    m_frm_option->setWaitServerReady();
    m_settings->endGroup();

    m_settings->beginGroup("test");
    m_testping = m_settings->value("testping", false).toBool();
    m_settings->endGroup();

    m_settings->beginGroup("Terminal");
//TODO
    m_settings->endGroup();
    m_settings->beginGroup("gps");
    m_OpenStreetMapTile = m_settings->value("OpenStreetMapTile", "https://tile.openstreetmap.org/{z}/{x}/{y}.png").toString();
    m_settings->endGroup();
}

void QIperfC::doClear()
{
    //clear all test date, config setting remain unchanged
    m_throughputview->doClear();
    m_TestStartTime = QDateTime();
    // m_tpplot->setStartTime(m_TestStartTime);
    m_qipconfig->clear();
    emit updateStatus("");
    emit updateStarttime(QDateTime());
    ui->actionShowLog->setEnabled(false);
}

void QIperfC::AddSerialView(QString mkey, SerialView *serialview, WSClient *wsc)
{
    //windows menu
    QAction *act = new QAction(QIcon(":/serial"), mkey, this);
    act->setData(ViewType::Serial);
    connect(act, &QAction::triggered, this, &QIperfC::showView);
    ui->menuWindows->addAction(act);

    m_views->addView(serialview, true);
    m_serialviews->insert(mkey, {serialview, wsc});
}

void QIperfC::AddSSHView(QString mkey, SSHView *sshview, WSClient *wsc)
{
    //windows menu
    QAction *act = new QAction(QIcon(":/ssh"), mkey, this);
    act->setData(ViewType::SSH);
    connect(act, &QAction::triggered, this, &QIperfC::showView);
    ui->menuWindows->addAction(act);

    m_views->addView(sshview, true);
    m_sshviews->insert(mkey, {sshview, wsc});

}

void QIperfC::onExport()
{
    if (m_TestStartTime.isValid()){
        //export test record to html file
        // QString templatefile = qApp->applicationDirPath()+QDir::separator()+"template"+QDir::separator()+"result.html";
        QString templatefile = ":/template/result.html";
        qDebug() << "templatefile: " << templatefile;

        QString path;
        if (!m_oldsavepath.isNull()){
            path = m_oldsavepath;
        }else {
            path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        }
        QString fileName = QFileDialog::getSaveFileName(this,
                                                        tr("Export Result to html"), path, tr(HTML_EXT_FILTER));
        QFileInfo fi(fileName);
        // QString img = fi.path() +QDir::separator()+ fi.baseName()+".png";
        QString ext = fi.suffix();
        if (ext.compare(HTML_EXT)!=0){
            fileName = fi.path() +QDir::separator()+ fi.baseName() + "."+ HTML_EXT;
        }
        QStringList pcs = m_throughputview->getPCs();
        // qDebug() << "pcs:" << pcs;
#if (DEBUG_EXPORT_HTML==1)
        eh = new ExportHtml(templatefile, fileName, m_TPExportWidth, m_TPExportHeigth, m_debugdlg);
#else
        eh = new ExportHtml(templatefile, fileName, m_TPExportWidth, m_TPExportHeigth);
#endif

        //debug ========================
#if (DEBUG_EXPORT_HTML==1)
        // Create the dialog
        m_debugdlg->setWindowTitle("Export to HTML");
        // Create a layout and add the widget to it
        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(eh);
        // Set the layout on the QDialog
        m_debugdlg->setLayout(layout);
        // Show the dialog
        m_debugdlg->show();
        //end debug ========================
#endif
        QString pcsinfo = m_qipconfig->getPCsInfo();
        if (pcsinfo.isEmpty()){
            pcsinfo = m_endpointmgr->getPCsInfo(pcs);
        }
        eh->setData(m_throughputview->getChilds(false), m_throughputview->toPixmap(m_TPExportWidth, m_TPExportHeigth), pcsinfo);
        eh->setTestTime(m_TestStartTime.toString(DATETIME_NOW_FORMAT));
        eh->setRawFilenames(m_qipconfig->getIperfRawFilenames());
        // TODO: DUT info. model, firmware ver, HW ver...
        // eh->procressData();
        eh->exporthtml();

    }else {
        qDebug() << "NO throughput record to Export : TestStartTime: " << m_TestStartTime.toString(DATETIME_NOW_FORMAT);
    }
}

void QIperfC::onWidthChanged(int width)
{
    m_TPExportWidth = width;
}

void QIperfC::onHeigthChanged(int heigth)
{
    m_TPExportHeigth = heigth;
}

void QIperfC::onShowGroup(bool bShow)
{
    m_throughputview->setShowGroupTotal(bShow);
}

void QIperfC::setShowGroup(bool bShow)
{
    m_TPGroup = bShow;
    //TODO: update m_frm_option's cb_TPGroup check status.
    m_dlgoption->setShowGroup(bShow);
}

void QIperfC::onUpdateTPUnit(QString sunit)
{
    m_TPUnit = sunit;
}

void QIperfC::onIgnoreWrongInterval(bool bIgnore)
{
    m_IgnoreWrongInterval = bIgnore;
    qDebug() << "onIgnoreWrongInterval:" << bIgnore;
    m_qipconfig->setIgnoreWrongInterval(m_IgnoreWrongInterval);
    //TODO: info all qiperfd Ignore Wrong Interval data on report iperf throughput?or just not show the wrong data??
}

void QIperfC::onUpdateOpenStreetMapTile(QString tile)
{
    m_OpenStreetMapTile = tile;
}

void QIperfC::onSerialOpened(QString refrow, QString serveraddress, QString serveraPort)
{
    if (m_serialviews->count() > refrow.toInt()){
        int index = refrow.toInt();
        QList<QString> keys = m_serialviews->keys();
        if (index >= 0 && index < keys.size()) {
            QString key = keys.at(index);
            // SerialView* sv = m_serialviews->value(key);
            SerialData sd = m_serialviews->value(key);
            sd.sv->setConfig(serveraddress, serveraPort.toInt());
            sd.sv->setLogFile(_logtofile, _logfilename, _logtimestemp, _logtimestempformat);
                // switch to view
            m_views->activateDock(sd.sv);
        }
    } else {
        qDebug() << refrow << " refrow out of index: " << m_serialviews;
    }
}
void QIperfC::onSerialClosed(QString idx)
{
    if (m_serialviews->contains(idx)){
        SerialData sd = m_serialviews->take(idx);
        QString cmd= QString("%1:%2").arg(CMD_SERIAL_DEL, idx);
        qInfo() << "onSerialClosed:" << cmd;
        sd.ws->sendText(cmd);
        sd.ws->deleteLater();
        sd.sv->deleteLater();
        for (auto it = m_serialviews->begin(); it != m_serialviews->end(); /* don't increment here */){
            if (it.key() == idx) {
                it = m_serialviews->erase(it);
            } else {
                ++it;
            }
        }
        foreach (auto *act, ui->menuWindows->actions()){
            if (act->text() == idx){
                qDebug() << "remove action menu";
                ui->menuWindows->removeAction(act);
            }
        }
    }else {
        qDebug() << "m_serialviews does not have " << idx;
    }
}

void QIperfC::onSSHOpened(QString refrow, QString serveraddress, QString serveraPort)
{
    if (m_sshviews->count() > refrow.toInt()){
        int index = refrow.toInt();
        QList<QString> keys = m_sshviews->keys();
        if (index >= 0 && index < keys.size()) {
            QString key = keys.at(index);
            SSHData sd = m_sshviews->value(key);
            sd.sv->setConfig(serveraddress, serveraPort.toInt());
            sd.sv->setLogFile(_logtofile, _logfilename, _logtimestemp, _logtimestempformat);
            // switch to view
            m_views->activateDock(sd.sv);
        }
    } else {
        qDebug() << refrow << "onSSHOpened refrow out of index: " << m_sshviews;
    }
}

void QIperfC::onSSHClosed(QString idx)
{
    qInfo() << "onSSHClosed: idx" << idx;
    if (m_sshviews->contains(idx)){
        SSHData sd = m_sshviews->take(idx);
        QString cmd= QString("%1:%2").arg(CMD_SSH_DEL, idx);
        qInfo() << "onSSHClosed:" << cmd;
        sd.ws->sendText(cmd);
        sd.ws->deleteLater();
        sd.sv->deleteLater();
        for (auto it = m_sshviews->begin(); it != m_sshviews->end(); /* don't increment here */) {
            if (it.key() == idx) {
                it = m_sshviews->erase(it);
            } else {
                ++it;
            }
        }
        foreach (auto *act, ui->menuWindows->actions()){
            if (act->text() == idx){
                qDebug() << "remove action menu";
                ui->menuWindows->removeAction(act);
            }
        }

    }else {
        qDebug() << "m_sshviews does not have " << idx;
    }
}

void QIperfC::showView()
{
    QAction* act = qobject_cast<QAction*>(sender());
    if (act != nullptr) {
        QString mkey = act->text();
        if (act->data() == ViewType::Serial){
            qDebug() << mkey << "Serial data:" << act->data().toString();
            if (m_serialviews->contains(mkey)){
                SerialData sd =  m_serialviews->value(mkey);
                m_views->activateDock(sd.sv);
            }else{
                qDebug() << " no " << mkey << " in " << m_serialviews;
            }
        }
        if (act->data() == ViewType::SSH){
            qDebug() << mkey << "SSH data:" << act->data().toString();
            if (m_sshviews->contains(mkey)){
                SSHData sd =  m_sshviews->value(mkey);
                m_views->activateDock(sd.sv);
            }else{
                qDebug() << " no " << mkey << " in " << m_sshviews;
            }
        }
    }
}

void QIperfC::onAutoLoadFile(QString idx, QString filename, QString savepath)
{
    //load file
    if (load(filename)){
        QString testtime = getNowString();
        m_smicroIdx = idx.toInt();
        onStart(false);
        // qDebug() << "m_smicroIdx:" << QString::number(m_smicroIdx);
        if (m_smicroIdx>=0) {
            QString tp="";
            QString lr="";
            m_throughputview->getTP(tp, lr);
            // qDebug() << "tp:" << tp << " lr:" << lr;
            emit reportTP(m_smicroIdx, tp.toDouble(), lr.toDouble());
        }
        QFileInfo f(filename);
        QString target = savepath + QDir::separator() + testtime + "_"+ f.fileName();
        qDebug() << "save to new file: " << target;
        save(target);
    }
}

void QIperfC::onDoNtpSync(QString target)
{
    QString s = "ws://"+target+":"+QString::number(QIPERFD_WSPORT);
    WSClient *ws = new WSClient(target, QUrl(s), "");
    connect(ws, &WSClient::ntpsynced, this, &QIperfC::onNtpsynced);
    int itimeout=20;
    bool bConnected=false;
    while (!bConnected && (itimeout>0)){
        bConnected = ws->isConnected();
        QThread::msleep(200);
        QCoreApplication::processEvents(QEventLoop::AllEvents);
        itimeout--;
    }
    if (bConnected){
        QString cmd=QString("%1:%2").arg(CMD_NTP_SYNC, getNowString());
        qint64 rc = ws->sendText(cmd);
        if (rc<=0){
            qDebug() << "send cmd Fail:" << cmd;
        }
        if (!m_ntpfail.contains(target)){
            m_ntpfail[target]=1;
        }else{
            m_ntpfail[target]=m_ntpfail[target]+1;
        }
    }else{
        qDebug() << "connect to ws: " + s + " Fail";
    }
}

void QIperfC::onNtpsynced(bool bOK, QString target)
{
    if (bOK){
        if (!m_ntps.contains(target)){
            qDebug() << "onNtpsynced:" << target;
            m_ntps.append(target);
        }
    }else {
        QMessageBox::information(this, "NOTICE",
                                 QString("%1 NTP time sync fail !").arg(target),
                                 QMessageBox::Ok);
    }
}

void QIperfC::onClearNtpStatus(QString target)
{
    if(m_ntpfail.contains(target)){
        qDebug() << "remove " << target << " from m_ntpfail";
        m_ntpfail.remove(target);
    }else{
        qDebug() << "onClearNtpStatus:" << m_ntpfail << " DO not have:" << target;
    }
}

void QIperfC::onRPC_result(const QVariant &result)
{
    qDebug() << "onRPC_result: " << result;
}

void QIperfC::onRPC_error(int code, const QString &message)
{
    qDebug() << "onRPC_error: (" << code << ")" << message;
}

void QIperfC::onUpdateDataPath(QString datapath)
{
    m_datapath = datapath;
    m_dlgrecord->setRootPath(datapath);
    m_fileserver->setRootPath(datapath);
    ui->actionShowLog->setEnabled(true);
}

void QIperfC::onProgress(QString filename, int currentlineno)
{
    if (currentlineno>0){
        onUpdateStatus("Procress "+filename+" line "+ QString::number(currentlineno));
    }else{
        onUpdateStatus("");
    }
}

void QIperfC::onAddSerial()
{
    m_dlgserial->setSerialData(m_endpointmgr->getSerials());
    if (m_dlgserial->exec()== QDialog::Accepted){
        QString managerip = m_dlgserial->getManagerIP();
        QString serailport = m_dlgserial->getSerialPort();
        QString mkey = managerip+":"+serailport;
        _logtofile=false;
        _logfilename = m_dlgserial->getLogFilename();
        if (!_logfilename.isEmpty()){
            _logtofile = true;
        }
        _logtimestemp=false;
        _logtimestempformat = m_dlgserial->getLogTimeStempFormat();
        if (!_logtimestempformat.isEmpty()){
            _logtimestemp = true;
        }

        if (!m_serialviews->contains(mkey)){
            long long idx = m_serialviews->count();
            QString serialcfg = m_dlgserial->getSerialCfg();
            qInfo() << "managerip: " << managerip << " serailport:" << serailport << " serialcfg:" << serialcfg;
            //
            QString url = "ws://"+managerip+":"+QString::number(QIPERFD_WSPORT);
            WSClient *wsc=new WSClient(managerip, QUrl(url), "");
            //TODO: when disconnected do waht?
            connect(wsc, &WSClient::serialopened, this, &QIperfC::onSerialOpened);
            //wait connect
            int timeout=0;
            while (!wsc->isConnected() && (timeout<30)){ // timeout 3 sec?
                // qDebug() << " wait WSClient connected";
                QThread::msleep(100);
                QCoreApplication::processEvents(QEventLoop::AllEvents);
                timeout++;
            }
            //ask remote create serialport and start tcp server on port
            QString sendstr = QString("%1:%2:%3:%4").arg(CMD_SERIAL_ADD,
                                                         QString::number(idx),
                                                         serailport, serialcfg);
            qInfo() << "onAddSerial sendstr: " << sendstr;
            wsc->sendText(sendstr);

            SerialView *serialview = new SerialView(mkey,
                                                    m_dlgoption->getFontName(),
                                                    m_dlgoption->getFontStyle(),
                                                    m_dlgoption->getFontSize());
            connect(serialview, &SerialView::closed, this, &QIperfC::onSerialClosed);
            AddSerialView(mkey, serialview, wsc);
        }else{
            qInfo() << "serialviews: " << mkey << " exist, show it";
            SerialData sd =  m_serialviews->value(mkey);
            m_views->activateDock(sd.sv);

        }
    }
}

void QIperfC::onAddSSH()
{
    if (m_dlgssh->exec()== QDialog::Accepted){
        QString managerip = m_dlgssh->getManagerIP();
        QString targetip = m_dlgssh->getTargetip();
        int targetport = m_dlgssh->getTargetport();
        QString mkey = managerip+":"+targetip+":"+QString::number(targetport);
        _logtofile=false;
        _logfilename = m_dlgssh->getLogFilename();
        if (!_logfilename.isEmpty()){
            _logtofile = true;
        }
        _logtimestemp=false;
        _logtimestempformat = m_dlgssh->getLogTimeStempFormat();
        if (!_logtimestempformat.isEmpty()){
            _logtimestemp = true;
        }
        if (!m_sshviews->contains(mkey)){
            long long idx = m_sshviews->count();
            QString sshcfg = m_dlgssh->getSSHCfg();
            // qInfo() << "managerip: " << managerip << " targetip:" << targetip
            //         << " targetport:" << QString::number(targetport);

            QString url = "ws://"+managerip+":"+QString::number(QIPERFD_WSPORT);
            WSClient *wsc=new WSClient(managerip, QUrl(url), "");
            //TODO: when disconnected do waht?
            connect(wsc, &WSClient::sshopened, this, &QIperfC::onSSHOpened);
            //wait connect
            int timeout=0;
            while (!wsc->isConnected() && (timeout<30)){ // timeout 3 sec?
                qDebug() << "wait WSClient connect to " << url;
                QThread::msleep(100);
                QCoreApplication::processEvents(QEventLoop::AllEvents);
                timeout++;
            }
            //ask remote create sshport and start tcp server on port
            //idx, sshTarget, sshPort, username, password, privateKeyFile, timeout
            QString sendstr = QString("%1:%2:%3:%4:%5").arg(CMD_SSH_ADD,
                                                         QString::number(idx),
                                                         targetip, QString::number(targetport),
                                                         sshcfg);
            wsc->sendText(sendstr);

            SSHView *sshview = new SSHView(mkey,
                                           m_dlgoption->getFontName(),
                                           m_dlgoption->getFontStyle(),
                                           m_dlgoption->getFontSize());
            connect(sshview, &SSHView::closed, this, &QIperfC::onSSHClosed);
            AddSSHView(mkey, sshview, wsc);
        }else{
            qInfo() << "sshview: " << mkey << " exist, show it";
            SSHData sd =  m_sshviews->value(mkey);
            m_views->activateDock(sd.sv);
        }
    }
}

void QIperfC::onGPScalc()
{
    dlg_gps = new DlgGpsCalc(m_settings);
    connect(this, &QIperfC::closeAll, dlg_gps, &DlgGpsCalc::close);
    dlg_gps->show();
}

void QIperfC::initActions()
{
    // init actions
    // file
    connect(ui->actionNew, &QAction::triggered, this, &QIperfC::onNew);
    connect(ui->actionOpen, &QAction::triggered, this, &QIperfC::onOpen);
    connect(ui->actionSave, &QAction::triggered, this, &QIperfC::onSave);
    ui->actionSave->setEnabled(false);
    connect(ui->actionIperf3Log, &QAction::triggered, this, &QIperfC::onImportIperf3Log);
    connect(ui->actionIperf2Log, &QAction::triggered, this, &QIperfC::onImportIperf2Log);
    connect(ui->actionExport, &QAction::triggered, this, &QIperfC::onExport);
    connect(ui->actionQuit, &QAction::triggered, this, &QIperfC::onQuit);
    // edit
    connect(ui->actionCopy, &QAction::triggered, this, &QIperfC::onCopy);
    connect(ui->actionCopyText, &QAction::triggered, this, &QIperfC::onCopyText);
    ui->actionPaste->setShortcutContext(Qt::ApplicationShortcut);//for shortcut Ctrl+V to work on ThroughputView
    connect(ui->actionPaste, &QAction::triggered, this, &QIperfC::onPaste);
    connect(ui->actionDelete, &QAction::triggered, this, &QIperfC::onDelete);
    // iperf
    connect(ui->actionAddIperf, &QAction::triggered, m_throughputview, &ThroughputView::onAddIperf);
    connect(ui->actionEdit, &QAction::triggered, m_throughputview, &ThroughputView::onPairEdit);
    connect(ui->actionSwap, &QAction::triggered, m_throughputview, &ThroughputView::onPairSwap);
    connect(ui->actionSwapIP, &QAction::triggered, m_throughputview, &ThroughputView::onPairSwapIP);
    // run
    connect(ui->actionStart, &QAction::triggered, this, &QIperfC::onStart);
    connect(ui->actionStop, &QAction::triggered, this, &QIperfC::onStop);
    connect(ui->actionClear, &QAction::triggered, this, &QIperfC::onClear);
    connect(ui->actionShowLog, &QAction::triggered, this, &QIperfC::onShowLog);
    // monitor
    // connect(ui->actionAddPing, &QAction::triggered, this, &QIperfC::onAddPing);
    connect(ui->actionAddSerial, &QAction::triggered, this, &QIperfC::onAddSerial);
    connect(ui->actionSSH, &QAction::triggered, this, &QIperfC::onAddSSH);
    if (!m_testping){
        ui->actionAddPing->setVisible(false);
    }
    connect(ui->actionAddPing, &QAction::triggered, this, &QIperfC::onAddPing);
    connect(ui->actionWlanSTA, &QAction::triggered, this, &QIperfC::onWlanSTA);
    //tools
    connect(ui->actionGPScalc, &QAction::triggered, this, &QIperfC::onGPScalc);
    //option
    connect(ui->actionConfig, &QAction::triggered, this, &QIperfC::onConfig);
    // auto
    connect(ui->actionSimple, &QAction::triggered, this, &QIperfC::onSimpleMicro);
    //TODO: actionMacro, more complex with other control
    //help
    connect(ui->actionAbout, &QAction::triggered, this, &QIperfC::onAbout);
    connect(ui->actionShowDebugLog, &QAction::triggered, this, &QIperfC::onShowDebugLog);
    //test
    // connect(ui->actionTest, &QAction::triggered, this, &QIperfC::onTest);

}

void QIperfC::initToolbar()
{
    QMenu *menuAdd = new QMenu(this);
    menuAdd->addAction(ui->actionAddIperf);
    // menuAdd->addAction(ui->actionAddPing);

    ui->actionAdd->setMenu(menuAdd);

    ui->toolBar->insertAction(ui->actionEdit, ui->actionAdd);
}

void QIperfC::initStatusbar()
{
    m_start_label = new QLabel();
    m_start_label->setFrameStyle(static_cast<int>(QFrame::StyledPanel) | static_cast<int>(QFrame::Sunken));
    ui->statusbar->addWidget(m_start_label, 1);
    connect(this , &QIperfC::updateStarttime, this,  &QIperfC::onUpdateStarttime);

    m_status_label = new QLabel();
    m_status_label->setFrameStyle(static_cast<int>(QFrame::StyledPanel) | static_cast<int>(QFrame::Sunken));
    ui->statusbar->addWidget(m_status_label, 2);
    connect(this , &QIperfC::updateStatus, this,  &QIperfC::onUpdateStatus);

    // statusbar of endpints
    m_label_qiperfd = new QLabel(this);
    m_label_qiperfd->installEventFilter(this);
//TODO: double click
    m_label_qiperfd->setText(QString(QIPERFD_NAME)+":0");
    m_label_qiperfd->setFrameStyle(static_cast<int>(QFrame::Box) | static_cast<int>(QFrame::Sunken));
//    m_endpoint_label->setTextFormat(Qt::RichText);
//    m_endpoint_label->setOpenExternalLinks(true);
    ui->statusbar->addPermanentWidget(m_label_qiperfd);
    connect(this , &QIperfC::updateEndpointNum, this, &QIperfC::on_updateQIperfdNum);
}

void QIperfC::onUpdateStarttime(QDateTime stime)
{
    m_throughputview->setStartTime(stime);
    m_start_label->setText(stime.toString(DATETIME_NOW_FORMAT));
}

void QIperfC::onUpdateStatus(QString msg)
{
    m_status_label->setText(msg);
}

void QIperfC::onUpdateRunStatus(bool bStart)
{
    updateRunStatus(bStart);
}

void QIperfC::onUpdateActions(bool bStart, bool bStop, bool bClear)
{
    ui->actionStart->setEnabled(bStart);
    ui->actionStop->setEnabled(bStop);
    ui->actionClear->setEnabled(bClear);

}

void QIperfC::onUpdateActionsSave(bool bSave)
{
    ui->actionSave->setEnabled(bSave);
}

void QIperfC::onUpdateActionsEdit(bool bDel, bool bEdit, bool bSwap, bool bSwapIP)
{
    ui->actionDelete->setEnabled(bDel);
    ui->actionEdit->setEnabled(bEdit);
    ui->actionSwap->setEnabled(bSwap);
    ui->actionSwapIP->setEnabled(bSwapIP);
}

void QIperfC::on_updateQIperfdNum(int n)
{
    m_label_qiperfd->setText(QString(QIPERFD_NAME)+ ":" + QString::number(n));
}
