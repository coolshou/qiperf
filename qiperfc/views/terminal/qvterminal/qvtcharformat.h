#ifndef QVTCHARFORMAT_H
#define QVTCHARFORMAT_H

#include <QColor>
#include <QFont>
#include <QString>

class QVTCharFormat
{
public:
    QVTCharFormat();

    QFont* font();
    void setFont(const QFont &font);
    void updateFont(QString fontname, QString fontstyle, int fontsize);

    const QColor &foreground() const;
    void setForeground(const QColor &foreground);

    const QColor &background() const;
    void setBackground(const QColor &background);

    QFont::Style stringToFontStyle(const QString &styleStr);
protected:
    QFont _font;
    QColor _foreground;
    QColor _background;
};

#endif // QVTCHARFORMAT_H
