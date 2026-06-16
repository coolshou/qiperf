#ifndef DLGOPTION_H
#define DLGOPTION_H

//#include <QWidget>
#include <QDialog>
#include <QSettings>
#include <QCheckBox>

namespace Ui {
class DlgOption;
}

class dlgOption : public QDialog
{
    Q_OBJECT

public:
    explicit dlgOption(QSettings *cfg, QWidget *parent = nullptr);
//    explicit FormOption(QSettings *cfg, QStringList interfaces, QWidget *parent = nullptr);
    ~dlgOption() override;
    void loadcfg(QSettings *cfg);
    void updatecfg();
    void setWaitServerReady(int val);
    int getWaitServerReady();
    bool getShowManagerIPWarning();
    QString getFontName();
    QString getFontStyle();
    int getFontSize();
    // void setTPsize(int width, int heigth);
public slots:
    // void setShowGroup(bool bShow);
    void onSetTPGroupType(int grouptype);
    void onTPUnitChanged(QString sunit);
    void updateFontStyle(QString fontfamily);
    void onCpuCheckClicked(bool checked);
    void onMemCheckClicked(bool checked);
    void onCpuCheckIntervalValueChanged(int value);
    void onMemCheckIntervalValueChanged(int value);
signals:
    // void ipaddressUpdated(QString ipaddress, int port);
    void widthChanged(int width);
    void heigthChanged(int heigth);
    void showGroup(bool bShow);
    void setTPGroupType(int group);
    void IgnoreWrongInterval(bool ignore);
    void updateTPUnit(QString sunit);
    void updateOpenStreetMapTile(QString tile);
    void updateCpuCheckInterval(int interval);
    void updateMemCheckInterval(int interval);
protected:
    void changeEvent(QEvent *e) override;
    void hidetab(QString tabname);

private slots:
    void initFonts();
    QStringList getSysFontFamilies();
    void onReject();
    void onAccept();
    void onWidthChange(int width);
    void onHeigthChange(int heigth);
    void onStateChanged(int state);
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
    void onTPGroupTypeChange(QAbstractButton *button);
#else
    void onTPGroupTypeChange(int id);
#endif
    void onIgnoreWrongIntervalChanged(int state);
    QStringList getFontStyles(QString fontfamily);

private:
    Ui::DlgOption *ui;
    QSettings *m_cfg;
};

#endif // DLGOPTION_H
