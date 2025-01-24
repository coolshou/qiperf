#ifndef EXPORTHTML_H
#define EXPORTHTML_H

#include <QWidget>
#include <QWebEngineView>
#include <QString>
#include <QJsonArray>
#include <QStringList>

#include "tpmgr.h"
#include "tp.h"
#include "tpplot.h"
#include "iperfwrapper.h"

class ExportHtml : public QWidget
{
    Q_OBJECT
public:
    explicit ExportHtml(QString templatefile, QString savefile, int width, int heigth,
                        QWidget *parent = nullptr);
    ~ExportHtml();
    void loadhtml(QString filename);
    void AddDivRow(QString pId, QList<QString> values);
    void AddRawData(QString pId, QString name, QString data);
    void AddDivPng(QString pId, QString sImg);
    void save(QString filename);
    QString imageToBase64(const QImage &image, const char *format = "PNG");
    void setData(QList<TP *> tps,  QPixmap chat, QString pcs);
    void setRawFilenames(QStringList filenames);
    void setTestTime(QString time);
    void exporthtml();

public slots:
    void editTitleTag(QString title);
    void updateTitle(QString title);
    void onAddTag();
    void onLoadFinished(bool isOk);
    void procressData();

protected:
    // Reimplement the keyPressEvent to listen for F12 key
    void keyPressEvent(QKeyEvent *event) override;

signals:
    void ready();

private:
    QString m_templatefile;
    QString m_savefile;
    int m_width;
    int m_heigth;
    QString m_testtime;
    QWebEngineView *webView;
    // bool m_ok;
    // TPPlot *m_tpplot;
    QJsonArray m_pcs;
    QStringList m_iperf_raw_filenames; // iperf raw log filenames
    IperfWrapper *m_iperfwrapper;
    QString dirToDiv(QString dir);
    QWebEngineView *devTools;  // Separate view for developer tools
    QList<TP *> m_tps;
    QPixmap m_chat;
};

#endif // EXPORTHTML_H
