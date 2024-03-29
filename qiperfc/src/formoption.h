#ifndef FORMOPTION_H
#define FORMOPTION_H

#include <QWidget>
#include <QSettings>
namespace Ui {
class FormOption;
}

class FormOption : public QWidget
{
    Q_OBJECT

public:
    explicit FormOption(QSettings *cfg, QStringList interfaces, QWidget *parent = nullptr);
    ~FormOption() override;
    void loadcfg(QSettings *cfg);
    void updatecfg();
signals:
    void ipaddressUpdated(QString ipaddress, int port);

protected:
    void changeEvent(QEvent *e) override;

private slots:
    void on_pb_cancel_clicked();

    void on_pb_save_clicked();

private:
    Ui::FormOption *ui;
    QSettings *m_cfg;
};

#endif // FORMOPTION_H
