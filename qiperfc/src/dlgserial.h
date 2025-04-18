#ifndef DLGSERIAL_H
#define DLGSERIAL_H

#include <QDialog>
#include <QMap>
#include <QCloseEvent>
#include "port/portsetbox.h"

namespace Ui {
class DlgSerial;
}

class DlgSerial : public QDialog
{
    Q_OBJECT

public:
    explicit DlgSerial(QWidget *parent = nullptr);
    ~DlgSerial();
    void setSerialData(QMap<QString, QStringList> data);
    QString getManagerIP();
    QString getSerialPort();
    QString getSerialCfg();
    QString getLogFilename();
    QString getLogTimeStempFormat();

protected:
    void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onChangeSerial(QString text);
    void onPortConfig(bool checked);
    void onSelectLogFile(bool checked);
    void onTimeStempChanged(int checkstatus);
private:
    Ui::DlgSerial *ui;
    QMap<QString, QStringList> m_serials;
    PortSetBox *portconfig;
    QString oldpath;
};

#endif // DLGSERIAL_H
