#include "htmlwriter.h"
#include <QFile>


HtmlWriter::HtmlWriter(const QString &fileName, QObject *parent)
    : QObject{parent}, m_fileName(fileName)
{
    m_xmlWriter = new QXmlStreamWriter(&m_fileName);
    m_xmlWriter->setAutoFormatting(true);

}

bool HtmlWriter::writeHtml()
{
    QFile file(m_fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning("Cannot open file for writing: %s", qPrintable(file.errorString()));
        return false;
    }
    m_xmlWriter->writeStartDocument();
    // Start HTML
    m_xmlWriter->writeStartElement("html");
    writeHeader("Sample HTML Document");
    writeBody();
    // End HTML
    m_xmlWriter->writeEndElement(); // html
    m_xmlWriter->writeEndDocument();

    file.close();
    return true;
}

bool HtmlWriter::writeHeader(QString title)
{
    // Head section
    m_xmlWriter->writeStartElement("head");
    m_xmlWriter->writeTextElement("title", title);
    m_xmlWriter->writeStartElement("meta");
    m_xmlWriter->writeAttribute("charset", "UTF-8");
    m_xmlWriter->writeEndElement(); // meta
    m_xmlWriter->writeEndElement(); // head
    return true;
}

bool HtmlWriter::writeBody()
{
    // Body section
    m_xmlWriter->writeStartElement("body");

    // environment PC/data
    // config/cmd
    // plotchart image

    m_xmlWriter->writeStartElement("h1");
    m_xmlWriter->writeCharacters("Hello, World!");
    m_xmlWriter->writeEndElement(); // h1

    m_xmlWriter->writeStartElement("p");
    m_xmlWriter->writeCharacters("This is a sample HTML document created with QXmlStreamWriter.");
    m_xmlWriter->writeEndElement(); // p

    m_xmlWriter->writeStartElement("ul");
    for (int i = 1; i <= 3; ++i) {
        m_xmlWriter->writeStartElement("li");
        m_xmlWriter->writeCharacters(QString("List item %1").arg(i));
        m_xmlWriter->writeEndElement(); // li
    }
    m_xmlWriter->writeEndElement(); // ul

    m_xmlWriter->writeEndElement(); // body

    return true;
}
