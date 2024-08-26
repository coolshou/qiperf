#ifndef ICMPWRAPPER_H
#define ICMPWRAPPER_H

#include <QObject>
#include <QMap>

#ifndef _GNU_SOURCE
    #define _GNU_SOURCE /* for additional type definitions */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>

#ifdef _WIN32
#ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0601 /* for inet_XtoY functions on MinGW */
#endif

#include <process.h>  /* _getpid() */
#include <winsock2.h>
#include <ws2tcpip.h> /* getaddrinfo() */
#include <mswsock.h>  /* WSARecvMsg() */

#undef CMSG_SPACE
#define CMSG_SPACE WSA_CMSG_SPACE
#undef CMSG_FIRSTHDR
#define CMSG_FIRSTHDR WSA_CMSG_FIRSTHDR
#undef CMSG_NXTHDR
#define CMSG_NXTHDR WSA_CMSG_NXTHDR
#undef CMSG_DATA
#define CMSG_DATA WSA_CMSG_DATA

typedef SOCKET socket_t;
typedef WSAMSG msghdr_t;
typedef WSACMSGHDR cmsghdr_t;

/*
 * Pointer to the WSARecvMsg() function. It must be obtained at runtime...
 */
static LPFN_WSARECVMSG WSARecvMsg;

#else /* _WIN32 */

#ifdef __APPLE__
    #define __APPLE_USE_RFC_3542 /* for IPv6 definitions on Apple platforms */
#endif

#include <errno.h>
#include <fcntl.h>            /* fcntl() */
#include <netdb.h>            /* getaddrinfo() */
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>        /* inet_XtoY() */
#include <netinet/in.h>       /* IPPROTO_ICMP */
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>  /* struct icmp */
//#include <netinet/icmp6.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

typedef int socket_t;
typedef struct msghdr msghdr_t;
typedef struct cmsghdr cmsghdr_t;

#endif /* !_WIN32 */

#define IP_VERSION_ANY 0
#define IP_V4 4
#define IP_V6 6

#define ICMP_HEADER_LENGTH 8
#define MESSAGE_BUFFER_SIZE 1024
#define ICMP_PAYLOAD_SIZE 32  // Define the size of the ICMP payload buffer

#ifndef ICMP_ECHO
    #define ICMP_ECHO 8
#endif
#ifndef ICMP_ECHO6
    #define ICMP6_ECHO 128
#endif
#ifndef ICMP_ECHO_REPLY
    #define ICMP_ECHO_REPLY 0
#endif
#ifndef ICMP_ECHO_REPLY6
    #define ICMP6_ECHO_REPLY 129
#endif

#define REQUEST_TIMEOUT 1000000  //microsecond, us => 1sec
#define REQUEST_INTERVAL 1000000  //microsecond, us => 1sec

#ifdef _WIN32
    #define socket(af, type, protocol) \
        WSASocketW(af, type, protocol, NULL, 0, 0)
    #define close_socket closesocket
    #define getpid _getpid
    #define usleep(usec) Sleep((DWORD)((usec) / 1000))
#else
    #define close_socket close
#endif

#pragma pack(push, 1)

#if defined _WIN32 || defined __CYGWIN__

#if defined _MSC_VER || defined __MINGW32__
    typedef unsigned __int8 uint8_t;
    typedef unsigned __int16 uint16_t;
    typedef unsigned __int32 uint32_t;
    typedef unsigned __int64 uint64_t;
    #ifndef EAI_SYSTEM
        #define EAI_SYSTEM	  -11
    #endif
#endif

struct icmp {
    uint8_t icmp_type;
    uint8_t icmp_code;
    uint16_t icmp_cksum;
    uint16_t icmp_id;
    uint16_t icmp_seq;
};

#endif /* _WIN32 || __CYGWIN__ */

struct ip6_pseudo_hdr {
    struct in6_addr src;
    struct in6_addr dst;
    uint8_t unused1[2];
    uint16_t plen;
    uint8_t unused2[3];
    uint8_t nxt;
};

struct icmp6_packet {
    struct ip6_pseudo_hdr ip6_hdr;
    struct icmp icmp;
};

#pragma pack(pop)


class IcmpWrapper : public QObject
{
    Q_OBJECT
public:
    explicit IcmpWrapper(int idx, QString target, quint64 count=4, uint64_t timeout=3,
                         uint interval=1, uint packetsize=64, QString source="",
                         int ttl=64, QObject *parent = nullptr);
    int pingHost(QString &shostname, uint16_t id);
    void getTTLs();
    void start();
    void stop();
    void work();
#ifdef _WIN32
    static void init_winsock_lib(void);
    static void init_winsock_extensions(socket_t sockfd);
#endif

public slots:
    void onStarted();
    void onStoped(int idx);
    void onResponseTime(uint16_t seq, double responseTime, const char *checksum=nullptr);

signals:
    void started();
    void finished(int idx);
    void icmpResponseTime(uint16_t seq, double responseTime, const char *checksum=nullptr);
    void icmpResponse(const QString& message);
    void errorResponse(const QString& message);

private:
    int m_idx;
    bool running;
    quint64 m_seq;
    QString m_target;
    quint64 m_count; // max ping
    uint64_t m_timeout; // TODO
    uint m_interval;
    uint m_packetsize;
    QString m_source;
    int m_ttl;
    int ip_version; // TODO
    QMap<uint16_t, double> m_results; // idx, response Time: -1 fail
    // TODO: size
    int showtimestemp = 0;
    const char *timestempformat = NULL;
    char addr_str[INET6_ADDRSTRLEN] = "<unknown>";;
    QByteArray createIcmpPacket(int packet_id, int sequence);
    uint16_t calculateChecksum(const char *data, int len);
    unsigned short calculateChecksum(void *b, int len);
    void current_time(const char *timestempformat);
};

#endif // ICMPWRAPPER_H
