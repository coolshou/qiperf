#include "qvtline.h"

QVTLine::QVTLine()
{
}

void QVTLine::append(const QVTChar &c, int position)
{
    if (position < 0) {
        _chars.append(c);
    } else if (position >= _chars.count()) {
        QVTChar spc = c;
        spc.setC(' ');
        for (int w = position - _chars.count(); w > 0; --w) {
            _chars.append(spc);
        }
        _chars.append(c);
    } else {
        _chars[position] = c;
    }
}

void QVTLine::reduce(int position)
{
    if (position >= 0 && position < _chars.count()) {
        _chars.remove(position,1);
    }
}

const QVector<QVTChar> &QVTLine::chars() const
{
    return _chars;
}

void QVTLine::reserve(int size)
{
    _chars.resize(size);
}

int QVTLine::size() const
{
    return _chars.size();
}

QString QVTLine::text() const
{
    QString text;
    for (const QVTChar &c : _chars)
    {
        text.append(c.c());
    }
    return text;
}

QString QVTLine::mid(qsizetype position, qsizetype n) const
{
    if (position >= _chars.size() || position < 0)
    {
        return QString();
    }

    qsizetype size = n;
    if (position + size > _chars.size())
    {
        size = _chars.size() - position;
    }

    QString text;
    for (qsizetype col = position; col < position + size; col++)
    {
        text.append(_chars[col].c());
    }
    return text;

}

QString QVTLine::left(qsizetype position)
{
    if (position >= _chars.size() || position < 0)
    {
        position = _chars.size();
    }
    QString text;
    for (qsizetype col = 0; col < position ; col++)
    {
        text.append(_chars[col].c());
    }
    return text;
}
