#ifndef SIGHANDLER_H
#define SIGHANDLER_H

#include <QObject>
#include <QSocketNotifier>

#if defined(Q_OS_LINUX)
#include <signal.h>

static int setup_unix_signal_handlers();

class SigHandler : public QObject
{
    Q_OBJECT

  public:
    SigHandler(QObject *parent = nullptr);
    ~SigHandler() override;

    // Unix signal handlers.
    static void intSignalHandler(int unused);
    static void hupSignalHandler(int unused);
    static void termSignalHandler(int unused);

  public slots:
    // Qt signal handlers.
    void handleSigInt();
    void handleSigHup();
    void handleSigTerm();
  signals:
    void sigTERM();
    void sigHUP();
    void sigINT();

  private:
    static int sigintFd[2]; //SIGINT, Ctrl+C
    static int sighupFd[2]; //SIGHUP
    static int sigtermFd[2]; //SIGTERM

    QSocketNotifier *snInt;
    QSocketNotifier *snHup;
    QSocketNotifier *snTerm;
};

static int setup_unix_signal_handlers()
{
    struct sigaction interrupt, hup, term;

    interrupt.sa_handler = SigHandler::intSignalHandler;
    sigemptyset(&interrupt.sa_mask);
    interrupt.sa_flags = 0;
    interrupt.sa_flags |= SA_RESTART;

    if (sigaction(SIGINT, &interrupt, nullptr))
        return 1;

    hup.sa_handler = SigHandler::hupSignalHandler;
    sigemptyset(&hup.sa_mask);
    hup.sa_flags = 0;
    hup.sa_flags |= SA_RESTART;

    if (sigaction(SIGHUP, &hup, nullptr))
        return 2;

    term.sa_handler = SigHandler::termSignalHandler;
    sigemptyset(&term.sa_mask);
    term.sa_flags = 0;
    term.sa_flags |= SA_RESTART;

    if (sigaction(SIGTERM, &term, nullptr))
        return 3;

    return 0;
}
#endif

#endif // SIGHANDLER_H
