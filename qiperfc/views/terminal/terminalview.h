#ifndef TERMINALVIEW_H
#define TERMINALVIEW_H

#include "../abstractview.h"

class QVTerminal;

class TerminalView : public AbstractView
{
    Q_OBJECT

public:
    explicit TerminalView(QWidget *parent = nullptr);
    ~TerminalView() override;

    QString title() override { return tr("Terminal"); }
    QString iid() override { return "terminal"; }
    void loadSettings(QSettings *config) override;
    void receiveData(const QByteArray &array) override;
    void setEnabled(bool enabled) override;
    void clear() override;
    void setLogFile(bool logtofile, QString logfilename, bool logtimestemp, QString timestempformat);

private slots:
    void sendData(const QString &string);

private:
    QVTerminal *m_term;
};

#endif // TERMINALVIEW_H
