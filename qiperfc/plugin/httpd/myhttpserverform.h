#ifndef MYHTTPSERVERFORM_H
#define MYHTTPSERVERFORM_H

#include <QWidget>
#include <QHostAddress>
#include <QString>

namespace Ui {
class MyHttpServerForm;
}

class MyHttpServerForm : public QWidget
{
    Q_OBJECT

public:
    explicit MyHttpServerForm(QWidget *parent = nullptr);
    ~MyHttpServerForm() override;
public slots:
    void onStarted();
    void onStoped();
    void onSelectRootPath(bool checked);
signals:
    void sigStart(quint16 port, QString rootpath=QString::fromUtf8("."), QHostAddress host=QHostAddress::Any);
    void sigStop();
    void sigRootPathChange(QString path);
private slots:
    void onStart(bool checked);
    void onRootPathChanged(QString path);
    void updateStatus(bool started);
private:
    Ui::MyHttpServerForm *ui;
};

#endif // MYHTTPSERVERFORM_H
