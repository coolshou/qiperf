#ifndef MYHTTPSERVERFORM_H
#define MYHTTPSERVERFORM_H

#include <QWidget>
#include <QHostAddress>
#include <QString>
#include <QSettings>
#include <QCloseEvent>

namespace Ui {
class MyHttpServerForm;
}

class MyHttpServerForm : public QWidget
{
    Q_OBJECT

public:
    explicit MyHttpServerForm(QSettings *cfg = nullptr, QWidget *parent = nullptr);
    ~MyHttpServerForm() override;
    void LoadCfg(QSettings *cfg = nullptr);
    void SaveCfg();
public slots:
    void onStarted();
    void onStoped();
    void onErrorNotice(QString err);
    void onSelectRootPath(bool checked);
    void onReflash(bool checked);
signals:
    void sigStart(quint16 port, QString rootpath=QString::fromUtf8("."), QHostAddress host=QHostAddress::Any);
    void sigStop();
    void sigRootPathChange(QString path);
protected:
    void closeEvent(QCloseEvent *event) override;
private slots:
    void onStart(bool checked);
    void onRootPathChanged(QString path);
    void updateStatus(bool started);
private:
    Ui::MyHttpServerForm *ui;
    QString mOldRootPath;
    QSettings *mCfg;
};

#endif // MYHTTPSERVERFORM_H
