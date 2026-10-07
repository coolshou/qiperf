#include "mainwindow.h"

#include <QApplication>
#include <csignal> // 引入訊號處理標頭檔

int main(int argc, char *argv[])
{
#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX) || defined(Q_OS_MACOS)
    std::signal(SIGPIPE, SIG_IGN);
#endif
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
