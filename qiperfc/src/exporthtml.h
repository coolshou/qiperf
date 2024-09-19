#ifndef EXPORTHTML_H
#define EXPORTHTML_H

#include <QWidget>
#include <QWebEngineView>
#include <QString>

#include "tpmgr.h"
#include "tp.h"
#include "tpplot.h"

class ExportHtml : public QWidget
{
    Q_OBJECT
public:
    explicit ExportHtml(QString templatefile, QWidget *parent = nullptr);
    ~ExportHtml();
    void loadhtml(QString filename);
    void AddDivRow(QString pId, QList<QString> values);
    void save(QString filename);
    QString imageToBase64(const QImage &image, const char *format = "PNG");
    void processdata(TPMgr *tpmgr, TPPlot *m_tpplot);

public slots:
    void editTitleTag();
    void onAddTag();

signals:
private:
    QString m_templatefile;
    QWebEngineView *webView;
};

#endif // EXPORTHTML_H
