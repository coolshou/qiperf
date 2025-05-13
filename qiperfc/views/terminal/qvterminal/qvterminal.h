#ifndef QVTERMINAL_H
#define QVTERMINAL_H

#include <QAbstractScrollArea>
#include <QAction>
#include <QTimer>
#include <QFile>

#include "qvtlayout.h"

class QVTerminal : public QAbstractScrollArea
{
    Q_OBJECT
public:
    explicit QVTerminal(QWidget *parent = nullptr);
    ~QVTerminal() override;

    void setIODevice(QIODevice *device);

    // style
    QVTCharFormat* format();
    void setFormat(const QVTCharFormat &format);

    // mode
    bool echo() const;
    void setEcho(bool echo);

    bool crlf() const;
    void setCrlf(bool crlf);
    // log file
    void setLogFile(bool logtofile, QString logfilename, bool logtimestemp, QString timestempformat);
    void closelogfile();
    QPoint posToCursor(const QPoint &cursorPos) const;
signals:
    void transmitData(const QByteArray &data);

public slots:
    void writeData(QByteArray data);

    void paste();
    void appendData(const QByteArray &data);

protected slots:
    void read();
    void appendString(QString str);
    void reduceString(int position);
    void toggleCursor();
    void clearToEnd();
protected:
    virtual bool event(QEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
#ifndef QT_NO_CONTEXTMENU
    virtual void contextMenuEvent(QContextMenuEvent *event) override;
#endif // QT_NO_CONTEXTMENU

    // QAbstractScrollArea interface
protected:
    virtual bool viewportEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    QColor vt100color(char c);

    QAction *_pasteAction;
private:
    void formatChar(const QChar &c);
    void moveCursor(int xpos, int ypos);
    QByteArray insertTimeStemp(QByteArray data);
    bool isRowInSelection(int row) const;
    void copySelectedText();
private:
    QIODevice *_device;
    // parser
    enum State {
        Text,
        Escape,
        Format,
        ResetFont
    };
    State _state;
    int _formatValue;
    // cursor
    QVTCharFormat _format;
    QVTCharFormat _curentFormat;
    int _cw, _ch, _cascent;
    QPoint _cursorPos;
    QTimer _cursorTimer;
    bool _cvisible;
    // select
    bool _selecting = true;
    QPoint _startCursorSelectPos;
    QPoint _startSelectPos;
    QPoint _endSelectPos;
    // data
    QVTLayout *_layout;
    // mode
    bool _echo;
    bool _crlf;
    // log
    bool _logtofile;
    QString _logfilename;
    QFile *_logfile;
    bool  _logtimestemp;
    QString _logtimestempformat;
    static const int xMargin = 3;
    static const int yMargin = 3;
};

#endif // QVTERMINAL_H
