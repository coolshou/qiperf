#ifndef SSHVIEW_H
#define SSHVIEW_H

#include <QWidget>
#include "../views/abstractview.h"
#include "../src/port/tcpudpport.h"
#include "../views/terminal/terminalview.h"

namespace Ui {
class SSHView;
}

class SSHView : public AbstractView
{
    Q_OBJECT

public:
    explicit SSHView(QString title, QWidget *parent = nullptr);
    ~SSHView() override;
    QString title() override;
    QString iid() override;
    void setConfig(QString serveraddress, int portnumber, QString protocol = "TCP Client");
    void setLogFile(bool logtofile, QString logfilename, bool logtimestemp, QString timestempformat);

protected:
    void changeEvent(QEvent *e) override;
private slots:
    void readPortData();
    void writePortData(const QByteArray &array);
private:
    Ui::SSHView *ui;
    QString m_title;
    TcpUdpPort *m_currentport;
    TerminalView *m_termialview;
    bool m_pause = false;
    int m_rxCount = 0;
    int m_txCount = 0;
};

#endif // SSHVIEW_H
