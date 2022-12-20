#include <QCoreApplication>

#include "qiperfd.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QIperfd qiperfd = QIperfd();

    return app.exec();
}
