#ifndef DLGPING_H
#define DLGPING_H

#include <QDialog>

namespace Ui {
class DlgPing;
}

class DlgPing : public QDialog
{
    Q_OBJECT

public:
    explicit DlgPing(QWidget *parent = nullptr);
    ~DlgPing();
    QString getJsonstr();

private slots:
    void onAccepted();
    void onRejected();
protected:
    void changeEvent(QEvent *e) override;


private:
    bool isRequireConfigMet();
    Ui::DlgPing *ui;
};

#endif // DLGPING_H
