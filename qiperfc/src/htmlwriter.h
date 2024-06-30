#ifndef HTMLWRITER_H
#define HTMLWRITER_H

#include <QObject>
#include <QString>
#include <QXmlStreamWriter>

class HtmlWriter : public QObject
{
    Q_OBJECT
public:
    explicit HtmlWriter(const QString &fileName, QObject *parent = nullptr);
    bool writeHtml();
    bool writeHeader(QString title);
    bool writeBody();
signals:


private:
    QString m_fileName;
    QXmlStreamWriter *m_xmlWriter;
};

#endif // HTMLWRITER_H
