#ifndef DLGOPENSTREETMAP_H
#define DLGOPENSTREETMAP_H

#include <QDialog>
#include <QWebEngineView>

namespace Ui {
class DlgOpenStreetMap;
}

class DlgOpenStreetMap : public QDialog
{
    Q_OBJECT

public:
    explicit DlgOpenStreetMap(QWidget *parent = nullptr);
    ~DlgOpenStreetMap();
    void addMarker(QString lat, QString lon, QString label="lable");
    void clearMarker();
    void getMarkersCountAsync();
    void load(QString tile, QString lat="24.804162", QString lon="121.027736"); //24.804162, 121.027736
signals:
    void loadFinished(bool ok);
protected slots:
    void onSet(bool checked);
    void onAddMarker(bool checked);
    void onClearMarker(bool checked);
    void onLoadFinished(bool ok);
protected:
    void changeEvent(QEvent *e);

private:
    Ui::DlgOpenStreetMap *ui;
    QWebEngineView *view;
    int markerCount;
};

#endif // DLGOPENSTREETMAP_H
