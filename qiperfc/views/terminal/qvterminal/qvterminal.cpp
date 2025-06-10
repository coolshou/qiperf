#include "qvterminal.h"

#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#include <QAction>
#include <QDateTime>

#include <QDebug>
#include <QMenu>

QVTerminal::QVTerminal(QString fontname, QString fontstyle, int fontsize,
                       QWidget *parent)
    : QAbstractScrollArea(parent),
    m_fontname(fontname), m_fontstyle(fontstyle), m_fontsize(fontsize)
{
    _device = Q_NULLPTR;
    _logfile = new QFile();
    _logtofile = false;
    _logtimestemp = false;

    _cursorPos.setX(0);
    _cursorPos.setY(0);
    _cursorTimer.start(500); //blink the cursor
    _cvisible = true;
    connect(&_cursorTimer, &QTimer::timeout, this, &QVTerminal::toggleCursor);

    _echo = false;
    _crlf = false;
    _state = QVTerminal::Text;
    QVTCharFormat vtcf = QVTCharFormat();
    vtcf.updateFont(fontname, fontstyle, fontsize);
    setFormat(vtcf);
    _layout = new QVTLayout();
    _pasteAction = new QAction("Paste", this);
    _pasteAction->setShortcut(QKeySequence("Qt::SHIFT + Qt::Key_Insert"));
    _pasteAction->setShortcutContext(Qt::WidgetShortcut);
    connect(_pasteAction, &QAction::triggered, this, &QVTerminal::onPaste);
    addAction(_pasteAction);
}

QVTerminal::~QVTerminal()
{
    if(_logfile->isOpen()){
        closelogfile();
    }
}

void QVTerminal::setIODevice(QIODevice *device)
{
    _device = device;
    if (_device) {
        connect(_device, &QIODevice::readyRead, this, &QVTerminal::read);
        read();
    }
}

void QVTerminal::appendData(const QByteArray &data)
{
    QByteArray text;
    QByteArray utext; // utf8, big-5 ... text
    // bool isnotPrint=false;

    setUpdatesEnabled(false);
    if (_logtofile){
        QByteArray newdata = data;
        if(_logtimestemp){
            //TODO: append time stemp on begin of line
            if (newdata.contains("\n")){
                // qDebug() << "TODO: insert time stemp";
                newdata = insertTimeStemp(newdata);
            }
        }
        // log to file
        if (_logfile->isWritable()){
            _logfile->write(newdata);
            _logfile->flush();
        }else{
            qDebug() << "logfile not writeable " << _logfilename;
        }
    }
    // qDebug() << "appendData="+data;

    QByteArray::const_iterator it = data.cbegin();


    while (it != data.cend()) {
        QChar c = *it;
        if (utext.length()>0) {
            text.append(utext);
            utext.clear();
        }
        switch (_state) {
        case QVTerminal::Text:
            switch (c.unicode()) {
            case '\033':
                // \033即\x1B即ASCII 27，ESC
                // 匹配ESC[，即CSI	，Control Sequence Introducer
                appendString(text);
                text.clear();
                _state = QVTerminal::Escape;
                break;
            case '\r':
                appendString(text);
                text.clear();
                _cursorPos.setX(0);
                break;
            case '\n':
                appendString(text);
                text.clear();
                moveCursor(0, 1);
                break;
            case '\b':
                appendString(text);
                text.clear();
                moveCursor(-1, 0);
                break;
            case '\t':
                // qDebug() <<"tab";
                appendString(text);
                text.clear();
                break;
            default:
                if (!c.isPrint()) {
                    utext.append(c.unicode());
                }else {
                    QByteArray byteArray;
                    // byteArray.append(c.toLatin1());
                    byteArray.append(c.unicode());
                    text.append(byteArray);
                }
            }
            break;
        case QVTerminal::Escape:
            _formatValue = 0;
            if (c == '[') {
                _state = QVTerminal::Format;
            } else if (c == '(') {
                _state = QVTerminal::ResetFont;
            }
            break;
        case QVTerminal::Format:
            if (c >= '0' && c <= '9') {
                _formatValue = _formatValue * 10 + (c.cell() - '0');
            } else {
                if (c == ';' || c == 'm') {
                    if (_formatValue == 0) { // reset format
                        _curentFormat = _format;
                    } else if (_formatValue == 4) { // underline
                        _curentFormat.font()->setUnderline(true);
                    } else if (_formatValue == 7) { // reverse
                        QColor foreground = _curentFormat.foreground();
                        _curentFormat.setForeground(_curentFormat.background());
                        _curentFormat.setBackground(foreground);
                    } else if (_formatValue / 10 == 3) { // foreground
                        // TODO 90~97
                        _curentFormat.setForeground(vt100color(_formatValue % 10 + '0'));
                    } else if (_formatValue / 10 == 4) { // background
                        // TODO 100~107
                        _curentFormat.setBackground(vt100color(_formatValue % 10 + '0'));
                    }
                    if (c == ';') {
                        _formatValue = 0;
                        _state = QVTerminal::Format;
                    } else {
                        _state = QVTerminal::Text;
                    }
                }else if(c=='J') {
                    reduceString(-1);
                    _state = QVTerminal::Text;
                } else {
                    formatChar(c);
                    _state = QVTerminal::Text;
                }
            }
            break;
        case QVTerminal::ResetFont:
            _curentFormat = _format;
            _state = QVTerminal::Text;
            break;
        default:
            qDebug() << "TODO: _state:" << _state;
            // break;
        }
        it++;
    }
    appendString(text);

    verticalScrollBar()->setRange(0, _ch * (_layout->lineCount() + 1) - viewport()->size().height());
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
    setUpdatesEnabled(true);
    update();
}

void QVTerminal::formatChar(const QChar &c)
{
    switch (c.unicode()) {
    case 'K':
        clearToEnd();
        break;
    case 'A':
        moveCursor(0, -_formatValue);
        break;
    case 'B':
        moveCursor(0, _formatValue);
        break;
    case 'C':
        moveCursor(_formatValue, 0);
        break;
    case 'D':
        moveCursor(-_formatValue, 0);
        break;
    default:
        break;
    }
}

void QVTerminal::moveCursor(int xpos, int ypos)
{
    _cursorPos += QPoint(xpos, ypos);
    if (_cursorPos.x() < 0) {
        _cursorPos.setX(0);
    }
    if (_cursorPos.y() < 0) {
        _cursorPos.setY(0);
    }
    if (_cursorPos.y() >= _layout->lineCount()) {
        _layout->appendLine(_cursorPos.y() - _layout->lineCount() + 1);
    }
}

QByteArray QVTerminal::insertTimeStemp(QByteArray data)
{
    if(data.isNull() || data.isEmpty()){
        return data;
    }
    if (_logtimestemp){
        if (data.indexOf('\n')>=0){
            // Split and rebuild with timestamp after each line
            QByteArray timestamp = "[" + QDateTime::currentDateTime().toString(_logtimestempformat).toUtf8() + "] ";

            int pos = 0;
            int lastPos = 0;
            QByteArray modifiedData;
            //FIXME following will cause crash!!
            while ((pos = data.indexOf('\n', lastPos)) != -1) {
                modifiedData.append(data.mid(lastPos, pos - lastPos + 1));  // include newline
                modifiedData.append(timestamp);  // append the timestamp
                lastPos = pos + 1;  // advance past the newline
                QApplication::processEvents(QEventLoop::AllEvents);
            }
            // append the rest of the data after the last newline
            modifiedData.append(data.mid(lastPos));
            return modifiedData;
        }else{
            return data;
        }
    }else {
        return data;
    }
}

bool QVTerminal::isRowInSelection(int row) const
{
    int y1 = _startSelectPos.y();
    int y2 = _endSelectPos.y();
    if (y1 > y2) std::swap(y1, y2);
    return _selecting && (row >= y1 && row <= y2);
}

void QVTerminal::copySelectedText()
{
    if (_layout->lineCount()<=0)
        return;

    int y1 = _startSelectPos.y();
    int y2 = _endSelectPos.y();
    int x1 = _startSelectPos.x();
    int x2 = _endSelectPos.x();

    // Ensure proper order
    if (y1 > y2 || (y1 == y2 && x1 > x2)) {
        std::swap(y1, y2);
        std::swap(x1, x2);
    }

    QStringList selectedLines;

    for (int row = y1; row <= y2 && row < _layout->lineCount(); ++row) {
        // QString line = lines[row];
        QVTLine line = _layout->lineAt(row);
        if (row == y1 && row == y2) {
            selectedLines << line.mid(x1, x2 - x1);
        } else if (row == y1) {
            selectedLines << line.mid(x1);
        } else if (row == y2) {
            selectedLines << line.left(x2);
        } else {
            selectedLines << line.text();
        }
    }

    QString selectedText = selectedLines.join('\n');
    QGuiApplication::clipboard()->setText(selectedText);
}

void QVTerminal::onCopy()
{
    copySelectedText();
}

void QVTerminal::onPaste()
{
    QByteArray data;
    data.append(QApplication::clipboard()->text().toUtf8());
    emit transmitData(data);
}

void QVTerminal::onDelete()
{
    //TODO: implement Delete
}

void QVTerminal::onCopyText()
{
    onCopy();
}

QColor QVTerminal::vt100color(char c)
{
    switch (c) {
    case '1': return QColor(Qt::red);
    case '2': return QColor(Qt::green);
    case '3': return QColor(Qt::yellow);
    case '4': return QColor(Qt::blue);
    case '5': return QColor(Qt::magenta);
    case '6': return QColor(Qt::cyan);
    case '7': return QColor(Qt::white);
    default:  return QColor(Qt::black);
    }
}

void QVTerminal::read()
{
    if (!_device)
        return;
    if (_device->isReadable())
        appendData(_device->readAll());
}

void QVTerminal::appendString(QString str)
{
    foreach (QChar c, str) {
        QVTChar termChar(c, _curentFormat);
        _layout->lineAt(_cursorPos.y()).append(termChar, _cursorPos.x());
        _cursorPos.setX(_cursorPos.x() + 1);
    }
}

void QVTerminal::reduceString(int mode)
{
    if(mode>0)
         _layout->lineAt(_cursorPos.y()).reduce(_cursorPos.x()+1);
    else{
        _layout->lineAt(_cursorPos.y()).reduce(_cursorPos.x());
    //   _cursorPos.setX(_cursorPos.x() -1);
    }
}

void QVTerminal::toggleCursor()
{
    _cvisible = !_cvisible;
    viewport()->update();
}

void QVTerminal::clearToEnd()
{
    _layout->lineAt(_cursorPos.y()).reserve(_cursorPos.x());
}

bool QVTerminal::crlf() const
{
    return _crlf;
}

void QVTerminal::setCrlf(bool crlf)
{
    _crlf = crlf;
}

void QVTerminal::setLogFile(bool logtofile, QString logfilename, bool logtimestemp, QString timestempformat)
{
    closelogfile();

    _logtofile = logtofile;
    _logfilename = logfilename;
    _logtimestemp = logtimestemp;
    _logtimestempformat = timestempformat;
    if (_logtofile){
        _logfile->setFileName(_logfilename);
        _logfile->open(QIODevice::Append);
    }
}

void QVTerminal::closelogfile()
{
    if(_logfile->isOpen()){
        _logfile->flush();
        _logfile->close();
    }
}

QPoint QVTerminal::posToCursor(const QPoint &cursorPos) const
{
    return QPoint((cursorPos.x() - xMargin) / _cw, (cursorPos.y() - yMargin + verticalScrollBar()->value()) / _ch);
}

void QVTerminal::writeData(QByteArray data)
{
    if (_device){
        //FIXME: right click-> paste => cause APP crash?
        _device->write(data);
        if (_echo) {
            appendData(data);
        }
    }else{
        qDebug() << "_device not init!!";
    }

}

bool QVTerminal::echo() const
{
    return _echo;
}

void QVTerminal::setEcho(bool echo)
{
    _echo = echo;
}

QVTCharFormat* QVTerminal::format()
{
    return &_format;
}

void QVTerminal::setFormat(const QVTCharFormat &format)
{
    _format = format;
    _curentFormat = format;
    QFontMetrics fm(*_format.font());
    _cw = fm.boundingRect('W').width(); // how to Decide the chinese font width
    qDebug() << "QVTerminal::setFormat: cw:" << QString::number(_cw);
    _ch = fm.height();
    _cascent = fm.ascent();
}

bool QVTerminal::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        keyPressEvent(static_cast<QKeyEvent*>(event));
        return true;
    }
    return QAbstractScrollArea::event(event);
}

void QVTerminal::keyPressEvent(QKeyEvent *event)
{
    QByteArray data;
    switch (event->key()) {
    case Qt::Key_Up:
        data.append("\033[A");
        break;
    case Qt::Key_Down:
        data.append("\033[B");
        break;
    case Qt::Key_Right:
        data.append("\033[C");
        break;
    case Qt::Key_Left:
        data.append("\033[D");
        break;
    case Qt::Key_Home:
        data.append('\x01');
        break;
    case Qt::Key_End:
        data.append('\x05');
        break;
    case Qt::Key_Tab:
        data.append('\t');
        break;
    case Qt::Key_Backspace:
        data.append('\b');
        break;
    // ref.PC-Style Function Keys
    // https://invisible-island.net/xterm/ctlseqs/ctlseqs.html
    case Qt::Key_Delete:
        data.append("\033[3~");
        break;
    case Qt::Key_Insert:
        data.append("\033[2~");
        break;
    case Qt::Key_PageUp:
        data.append("\033[5~");
        break;
    case Qt::Key_PageDown:
        data.append("\033[6~");
        break;
    case Qt::Key_Return:
        // data.append('\n');
        data.append('\r'); // for windows cmd
        break;
    default:
        data.append(event->text().toUtf8());
        QAbstractScrollArea::keyPressEvent(event);
    }
    emit transmitData(data);
}

void QVTerminal::paintEvent(QPaintEvent */* paintEvent */)
{
    QPainter p(viewport());
    p.setPen(QColor(187, 187, 187));
    p.setBrush(QColor(0x23, 0x26, 0x29));
    p.setFont(*_format.font());

    p.fillRect(viewport()->rect(), QColor(0x23, 0x26, 0x29));

    QPoint pos(0, 0);

    int firstLine = verticalScrollBar()->value() / _ch;
    int lastLine = viewport()->size().height() / _ch + firstLine;
    if (lastLine > _layout->lineCount()) {
        lastLine = _layout->lineCount();
    }

    QPoint curPos(_cursorPos.x() * _cw, (_cursorPos.y() - firstLine) * _ch);

    // draw cursor
    if (_cvisible) {
        p.fillRect(QRect(curPos, QSize(_cw, _ch)), _format.foreground());
    }
    // QFontMetrics fm = fontMetrics();
    // int charWidth = fm.horizontalAdvance('M');
    // int charHeight = fm.height();

    // draw text
    for (int row = firstLine; row < lastLine; row++) {
        // draw selection background
        QVTLine line = _layout->lineAt(row);
        int y = (row - firstLine) * _ch;

        if (isRowInSelection(row)) {
            int selStart = (row == _startSelectPos.y()) ? _startSelectPos.x() : 0;
            //TODO current line length
            int selEnd   = (row == _endSelectPos.y()) ? _endSelectPos.x() : line.size();

            if (selStart > selEnd) std::swap(selStart, selEnd);

            int x1 = selStart * _cw;
            int x2 = selEnd * _cw;

            p.fillRect(QRect(x1, y, x2 - x1, _ch), QColor(0x33, 0x99, 0xff));
        }

        pos.setX(0);
        for (auto vtc : _layout->lineAt(row).chars()) {
            p.setPen(pos == curPos ? vtc.background() : vtc.foreground());
            p.drawText(pos.x(), pos.y() + _cascent, vtc.c());
            //p.setBrush(QBrush());
            //p.drawRect(QRect(pos, QSize(_cw, _ch)));
            pos.setX(pos.x() + _cw); // TODO: adj chinese char width?
        }
        pos.setY(pos.y() + _ch); // another row
    }
}

void QVTerminal::resizeEvent(QResizeEvent */* event */)
{
    QScrollBar *bar = verticalScrollBar();
    int pos = bar->maximum() - bar->value();
    bar->setPageStep(_ch * 10);
    bar->setSingleStep(_ch);
    bar->setRange(0, _ch * (_layout->lineCount() + 1) - viewport()->size().height());
    bar->setValue(bar->maximum() - pos);
}

void QVTerminal::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        _selecting = true;
        _endSelectPos = QPoint();
        _startCursorSelectPos = posToCursor(event->pos());
        setMouseTracking(true);
    }
    if (event->button() == Qt::MiddleButton) {
        if (QApplication::clipboard()->supportsSelection()) {
            QByteArray data;
            data.append(QApplication::clipboard()->text(QClipboard::Selection).toUtf8());
            // writeData(data);
            emit transmitData(data);
        }
    }
    QWidget::mousePressEvent(event);
}

void QVTerminal::mouseMoveEvent(QMouseEvent *event)
{
    if (!_startCursorSelectPos.isNull())
    {
        if (_selecting) {
            _endSelectPos = posToCursor(event->pos());
            if ((_startCursorSelectPos.y() > _endSelectPos.y())
                || (_startCursorSelectPos.y() == _endSelectPos.y() && _startCursorSelectPos.x() > _endSelectPos.x()))
            {
                _startSelectPos = posToCursor(event->pos());
                _endSelectPos = _startCursorSelectPos;
            }
            else
            {
                _startSelectPos = _startCursorSelectPos;
                _endSelectPos = posToCursor(event->pos());
            }
            viewport()->update();
        }
    }
}

void QVTerminal::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (_startCursorSelectPos == _endSelectPos)
        {
            _startSelectPos = QPoint();
            _endSelectPos = QPoint();
        }
        _selecting = false;
        copySelectedText();
        _startCursorSelectPos = QPoint();
        viewport()->update();
        setMouseTracking(false);
    }
    QAbstractScrollArea::mouseReleaseEvent(event);
}

#ifndef QT_NO_CONTEXTMENU
void QVTerminal::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.addAction(_pasteAction);
    _pasteAction->setEnabled(!QApplication::clipboard()->text().isEmpty());
    menu.exec(event->globalPos());
}

#endif // QT_NO_CONTEXTMENU

bool QVTerminal::viewportEvent(QEvent *event)
{
    return QAbstractScrollArea::viewportEvent(event);
}

void QVTerminal::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    qDebug() << "QVTerminal closeEvent";
}
