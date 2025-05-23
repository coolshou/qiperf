#ifndef VIEWMANAGER_H
#define VIEWMANAGER_H

#include <QObject>
#include <QCloseEvent>
#include "throughputview.h"

class AbstractView;
class QAction;
class QSettings;
class QMainWindow;

class ViewManager : public QObject
{
    Q_OBJECT
public:
    explicit ViewManager(QString *docPath, ThroughputView *tpview, QMainWindow *window);
    ~ViewManager() override;

    void loadConfig(QSettings *config);
    void saveConfig(QSettings *config);
    void loadSettings(QSettings *config);
    void retranslate();
    void receiveData(const QByteArray &array);
    void setEnabled(bool enabled);
    void clear(void);
    void setFileAction(QAction *openAction, QAction *saveAction);
    void addView(AbstractView *view, bool closeable=false);
    void activateDock(AbstractView *view);
    AbstractView* findActiveView();

public slots:
    // void onAddTPdata(QString midx, QString sInterval, QString idx,
    //                QString value, QString unit, QString dir=nullptr,
    //                QString pkt_lost="", QString pkt_total="");
    // void onUpdateTPDatas(QString refrow, QVector<double> timedatas, QVector<double> valuedatas,
    //                      QVector<int> packetlosts, QVector<int> packettotals, QVector<double> lostrates);

signals:
    void transmitData(const QByteArray &);

protected slots:
    void onDockWidgetClose(QCloseEvent* event);
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void dispatchMessage(const QString &receiver, const QByteArray &message);
    void saveFile();
    void openFile();
    void onVisibilityChanged(bool visible);

private:
    QVector<AbstractView *> loadExtensions(const QString &path);


private:
    struct Hotspot {
        AbstractView *view;
        int postion;
    };
    QDockWidget *m_align = nullptr;
    // QVector<AbstractView *> *m_views;
    QMap<QString, AbstractView *> *m_views;
    QMap<AbstractView *, QDockWidget *> *m_docks;
    QString *m_docPath;
    ThroughputView *m_throughputview;
    QMainWindow *m_window;


};

#endif // VIEWMEDIATOR_H
