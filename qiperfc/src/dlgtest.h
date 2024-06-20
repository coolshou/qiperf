#ifndef DLGTEST_H
#define DLGTEST_H

#include <QDialog>
#include <QString>

namespace Ui {
class DlgTest;
}

class DlgTest : public QDialog
{
    Q_OBJECT

public:
    explicit DlgTest(QWidget *parent = nullptr);
    ~DlgTest();
    void append(QString msg);

public slots:
    void onTest();
protected:
    void changeEvent(QEvent *e);

private slots:
    void on_pb_stop_clicked();
    void on_pb_add_server_clicked();
    void on_pb_status_clicked();
private:
    Ui::DlgTest *ui;
};

#endif // DLGTEST_H
