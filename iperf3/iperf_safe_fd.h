#ifndef IPERF_SAFE_FD_H
#define IPERF_SAFE_FD_H

#include <sys/select.h>

// Safe replacement for FD_SET that guards against negative file descriptors
static inline void safe_fd_set(int fd, fd_set *set) {
    if (fd >= 0) {
        FD_SET(fd, set);
    }
}

// Safe replacement for FD_ISSET that returns 0 if the fd is invalid
static inline int safe_fd_isset(int fd, fd_set *set) {
    if (fd < 0) {
        return 0;
    }
    return FD_ISSET(fd, set);
}

#endif // IPERF_SAFE_FD_H
