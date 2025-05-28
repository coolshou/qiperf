#include "qvtcharformat.h"

QVTCharFormat::QVTCharFormat()
{
    QFont font;
    // font.setFamily("monospace");
    font.setFamily("Noto Mono");
    font.setStyle(QFont::StyleNormal);
    font.setStyleHint(QFont::Monospace); //Qt does not support style hints on X11 since this information is not provided by the window system.
    font.setPointSize(10);
    setFont(font);
    setForeground(QColor(187, 187, 187));
    setBackground(QColor(0, 0, 0));
}

QFont::Style QVTCharFormat::stringToFontStyle(const QString &styleStr) {
    QString s = styleStr.trimmed().toLower();

    if (s == "italic")  return QFont::StyleItalic;
    if (s == "oblique") return QFont::StyleOblique;
    return QFont::StyleNormal; // default fallback
}

QFont* QVTCharFormat::font()
{
    return &_font;
}

void QVTCharFormat::setFont(const QFont &font)
{
    _font = font;
}

void QVTCharFormat::updateFont(QString fontname, QString fontstyle, int fontsize)
{
    _font.setFamily(fontname);
    _font.setStyle(stringToFontStyle(fontstyle));
    _font.setPointSize(fontsize);
}

const QColor &QVTCharFormat::foreground() const
{
    return _foreground;
}

void QVTCharFormat::setForeground(const QColor &foreground)
{
    _foreground = foreground;
}

const QColor &QVTCharFormat::background() const
{
    return _background;
}

void QVTCharFormat::setBackground(const QColor &background)
{
    _background = background;
}
