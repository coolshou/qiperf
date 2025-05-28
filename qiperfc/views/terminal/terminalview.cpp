#include "terminalview.h"
#include "qvterminal/qvterminal.h"
#include <QHBoxLayout>
#include <QTextCodec>
#include <QSettings>
#include <QtGlobal>

#include <QDebug>

TerminalView::TerminalView(QString fontname, QString fontstyle, int fontsize,
                           QWidget *parent)
    : AbstractView(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    m_term = new QVTerminal(fontname, fontstyle, fontsize, this);
    layout->addWidget(m_term);
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
    layout->setContentsMargins(2,2,2,2);
#else
    layout->setMargin(2);
#endif
    this->setLayout(layout);
    connect(m_term, SIGNAL(transmitData(QByteArray)), this, SIGNAL(transmitData(QByteArray)));
}

TerminalView::~TerminalView()
{

}

void TerminalView::loadSettings(QSettings *config)
{
    QString fontFamily("'" + config->value("FontFamily").toString().replace("+", "','") + "'");
    int fontSize = config->value("FontSize").toInt();

    fontSize = fontSize < 6 ? 10 : fontSize;
    //m_termView->setStyleSheet(BASE_STYLE
    //                          "font-family: " + fontFamily + ";" +
    //                          "font-size: " + QString::number(fontSize) + "pt;");
}

void TerminalView::sendData(const QString &string)
{
    // QTextCodec *code = QTextCodec::codecForName("GB-2312");
    QTextCodec *code = QTextCodec::codecForName("UTF-8");
    QByteArray array = code->fromUnicode(string);
    qDebug() << "send: " << array;
    emit transmitData(array);
}

void TerminalView::receiveData(const QByteArray &array)
{
    m_term->appendData(array);
}

void TerminalView::setEnabled(bool enabled)
{
    m_term->setEnabled(enabled);
}

void TerminalView::clear()
{
    //m_term->clear();
}

void TerminalView::setLogFile(bool logtofile, QString logfilename, bool logtimestemp, QString timestempformat)
{
    m_term->setLogFile(logtofile, logfilename, logtimestemp, timestempformat);
}

void TerminalView::onCopy()
{
    m_term->onCopy();
}

void TerminalView::onPaste()
{
    m_term->onPaste();
}

void TerminalView::onDelete()
{
    m_term->onDelete();
}

void TerminalView::onCopyText()
{
    m_term->onCopyText();
}
