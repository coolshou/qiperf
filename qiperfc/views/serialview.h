#ifndef SERIALVIEW_H
#define SERIALVIEW_H

#include <QWidget>
#include "../views/abstractview.h"
#include "../src/port/serialport.h"
#include "../views/terminal/terminalview.h"

namespace Ui {
class SerialView;
}

class SerialView : public AbstractView
{
    Q_OBJECT

public:
    explicit SerialView(QString title, QWidget *parent = nullptr);
    ~SerialView();
    QString title() override;
    QString iid() override;

protected:
    void changeEvent(QEvent *e);

private slots:
    void readPortData();
    void writePortData(const QByteArray &array);

private:
    Ui::SerialView *ui;
    QString m_title;
    SerialPort *m_serialport;
    TerminalView *m_termialview;
    bool m_pause = false;
    int m_rxCount = 0;
    int m_txCount = 0;
};

#endif // SERIALVIEW_H
