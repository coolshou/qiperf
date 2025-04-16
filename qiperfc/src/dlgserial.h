#ifndef DLGSERIAL_H
#define DLGSERIAL_H

#include <QDialog>
#include <QMap>

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

protected:
    void changeEvent(QEvent *e);
private slots:
    void onChangeSerial(QString text);
    void onPortConfig(bool checked);

private:
    Ui::DlgSerial *ui;
    QMap<QString, QStringList> m_serials;
    PortSetBox *portconfig;
};

#endif // DLGSERIAL_H
