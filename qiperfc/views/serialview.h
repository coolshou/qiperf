#ifndef SERIALVIEW_H
#define SERIALVIEW_H

#include <QWidget>
#include <QCloseEvent>

#include "../views/abstractview.h"
#include "../src/port/abstractport.h"
#include "../src/port/serialport.h"
#include "../src/port/tcpudpport.h"
#include "../views/terminal/terminalview.h"

namespace Ui {
class SerialView;
}

class SerialView : public AbstractView
{
    Q_OBJECT

public:
    explicit SerialView(QString title, QWidget *parent = nullptr);
    ~SerialView() override;
    QString title() override;
    QString iid() override;
    void setConfig(QString serveraddress, int portnumber, QString protocol = "TCP Client");
    void setLogFile(bool logtofile, QString logfilename, bool logtimestemp, QString timestempformat);
public slots:
    void onCopy() override;
    void onPaste() override;
    void onDelete() override;
    void onCopyText() override;

// signals:
//     void closed(QString title);
protected:
    void changeEvent(QEvent *e) override;
    void closeEvent(QCloseEvent *event) override;
private slots:
    void readPortData();
    void writePortData(const QByteArray &array);

private:
    Ui::SerialView *ui;
    QString m_title;
    TcpUdpPort *m_currentport;
    TerminalView *m_termialview;
    bool m_pause = false;
    int m_rxCount = 0;
    int m_txCount = 0;
};

#endif // SERIALVIEW_H
