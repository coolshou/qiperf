#include "icmpwrapper.h"

// #include <QCoreApplication>
// #include <QEventLoop>
#include <QAbstractSocket>

#include "myfunc.h"

#include <QDebug>

/*
*  send ICMP packet requirt root/administrator right
*/



/**
 * Returns a timestamp with microsecond resolution.
 */
static uint64_t utime(void)
{
#ifdef _WIN32
    LARGE_INTEGER count;
    LARGE_INTEGER frequency;
    if (QueryPerformanceCounter(&count) == 0
        || QueryPerformanceFrequency(&frequency) == 0) {
        return 0;
    }
    return count.QuadPart * 1000000 / frequency.QuadPart;
#else
    struct timeval now;
    return gettimeofday(&now, NULL) != 0
        ? 0
        : now.tv_sec * 1000000 + now.tv_usec;
#endif
}



static uint16_t compute_checksum(const char *buf, size_t size)
{
    /* RFC 1071 - http://tools.ietf.org/html/rfc1071 */

    size_t i;
    uint64_t sum = 0;

    for (i = 0; i < size; i += 2) {
        sum += *(uint16_t *)buf;
        buf += 2;
    }
    if (size - i > 0)
        sum += *(uint8_t *)buf;

    while ((sum >> 16) != 0)
        sum = (sum & 0xffff) + (sum >> 16);

    return (uint16_t)~sum;
}



IcmpWrapper::IcmpWrapper(int idx,QString target, quint64 count, uint64_t timeout,
                         uint interval, uint packetsize, QString source,int ttl,
                         QObject *parent)
    : QObject{parent},m_idx(idx),
    m_target(target), m_count(count),m_timeout(timeout*REQUEST_TIMEOUT),
    m_interval(interval*REQUEST_INTERVAL), m_packetsize(packetsize), m_source(source),
    m_ttl(ttl)
{
    m_seq = 0;
    QObject::connect(this, &IcmpWrapper::started, this, &IcmpWrapper::onStarted);
    QObject::connect(this, &IcmpWrapper::finished, this, &IcmpWrapper::onStoped);
    QObject::connect(this, &IcmpWrapper::icmpResponseTime, this, &IcmpWrapper::onResponseTime);
    timestempformat=QString("%Y%m%d_%H:%M:%S").toStdString().c_str();
    running=true;
    int protocal=-1;
    if (!isValidIpAddress(m_target, protocal)){
        qDebug() << "InValid IpAddress:" << m_target;
    }
    if (protocal==QAbstractSocket::IPv4Protocol){
        ip_version = IP_V4;
    }else if(protocal==QAbstractSocket::IPv6Protocol){
        ip_version = IP_V6;
    }
}

void IcmpWrapper::start()
{    //run in thread
//    m_seq = 0;
    qDebug() << "start";
    uint16_t id = (uint16_t)getpid();
    int rc = pingHost(m_target, id);
    if (rc != 0){
        qDebug() << "Ping " << m_target << " Fail";
    }
    qDebug() << "finished";
    emit finished(m_idx);
}

void IcmpWrapper::stop()
{
    running = false;
}

void IcmpWrapper::work()
{
    int error;
    int sockfd;
    uint64_t start_time;
    uint64_t delay;
    struct sockaddr_in dest_addr;
    struct sockaddr_in6 dest_addr6;
#ifdef _WIN32
    init_winsock_lib();
#endif
    char errorMessage[256];
    if (ip_version == IP_V4){
        if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
#ifdef _MSC_VER
            // 使用 strerror_s 獲取錯誤訊息
            strerror_s(errorMessage, sizeof(errorMessage), errno);
#else
            strerror_r(errno, errorMessage, sizeof(errorMessage));
#endif
            emit errorResponse(QString("Socket IPv4 creation failed: %1").arg(errorMessage));
            return;
        }
    }else if(ip_version == IP_V6){
        if ((sockfd = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6)) < 0) {
#ifdef _MSC_VER
            // 使用 strerror_s 獲取錯誤訊息
            strerror_s(errorMessage, sizeof(errorMessage), errno);
#else
            strerror_r(errno, errorMessage, sizeof(errorMessage));
#endif
            emit errorResponse(QString("Socket IPv6 creation failed: %1").arg(errorMessage));
            return;
        }
    }else{
        emit errorResponse(QString("unknown protocal: %1").arg(ip_version));
        return;
    }

#ifdef _WIN32
    init_winsock_extensions(sockfd);
#endif
    /*
     * Switch the socket to non-blocking I/O mode. This allows us to implement
     * the timeout feature.
     */
#ifdef _WIN32
    {
        u_long opt_value = 1;
        if (ioctlsocket(sockfd, FIONBIO, &opt_value) != 0) {
            psockerror("ioctlsocket");
            emit errorResponse(QString("set socket ioctlsocket error"));
//            goto exit_error;
            return;
        }
    }
#else /* _WIN32 */
    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1) {
        psockerror("fcntl");
//        goto exit_error;
        emit errorResponse(QString("set socket fcntl error"));
        return;
    }
#endif /* !_WIN32 */
    if (ip_version == IP_V4){
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_addr.s_addr = inet_addr(m_target.toStdString().c_str());
    }else{
        //TODO: IPV6
       memset(&dest_addr6, 0, sizeof(dest_addr6));
//        dest_addr6.sin6_family = AF_INET6;
//        dest_addr6.sin6_addr = inet_addr(m_target.toStdString().c_str());

    }
    int packet_id = getpid() & 0xFFFF;
    int sequence = 0;

    while (running) {
        if (sequence>=(int)m_count){
            emit finished(m_idx);
            return;
        }
        // TODO: IPv6
        QByteArray packet = createIcmpPacket(packet_id, sequence);

        start_time = utime();
        error = sendto(sockfd, packet.data(), packet.size(), 0,
                       (struct sockaddr*)&dest_addr, sizeof(dest_addr));
        if (error <= 0) {
            emit errorResponse(QString("Failed to send packet: %1").arg(strerror(errno)));
            close_socket(sockfd);
            emit finished(m_idx);
            return;
        }

//        emit icmpResponse(QString("ICMP packet sent to %1").arg(m_target));

        for (;;) {
            // QCoreApplication::processEvents(QEventLoop::AllEvents);
            // Listen for a response
            struct sockaddr_in reply_addr;
            socklen_t addr_len = sizeof(reply_addr);
            char buffer[1024];

            int n = recvfrom(sockfd, buffer, sizeof(buffer), 0,
                             (struct sockaddr*)&reply_addr, &addr_len);
            delay = utime() - start_time;
            if (delay > m_timeout) {
                if (showtimestemp){
                    current_time(timestempformat);
                }
                //timeout
                emit icmpResponseTime(sequence, -1);
                goto next;
            }
            if (n > 0) {
                struct iphdr* ip_hdr = (struct iphdr*)buffer;
                struct icmphdr* icmp_hdr = (struct icmphdr*)(buffer + (ip_hdr->ihl * 4));
                /*
                 * Verify that this is indeed an echo reply packet.
                 */
                if (!(reply_addr.sin_family == AF_INET
                      && icmp_hdr->type == ICMP_ECHO_REPLY)
                    && !(reply_addr.sin_family == AF_INET6
                         && icmp_hdr->type == ICMP6_ECHO_REPLY)) {
                    emit errorResponse(QString("Received non-echo reply ICMP packet from %1")
                                                      .arg(inet_ntoa(reply_addr.sin_addr)));
                    continue;
                }
                /*
                 * Verify the ID and sequence number to make sure that the reply
                 * is associated with the current request.
                 */
                uint16_t reply_id = icmp_hdr->un.echo.id;
                int recv_sequence = ntohs(icmp_hdr->un.echo.sequence);
                if (reply_id != htons(packet_id) || recv_sequence != sequence) {
                    emit errorResponse(QString("Received reply ICMP packet with wrong sequence from %1, SEQ:%2")
                                                      .arg(inet_ntoa(reply_addr.sin_addr))
                                      .arg(recv_sequence));
                    continue;
                }
                // Calculate the checksum of the received ICMP header
                // TODO: IPv6
                int icmp_hdr_len = n - (ip_hdr->ihl * 4);
                unsigned short received_checksum = icmp_hdr->checksum;
                icmp_hdr->checksum = 0;  // Reset checksum field for calculation

                unsigned short calculated_checksum = calculateChecksum(icmp_hdr, icmp_hdr_len);

                if (received_checksum == calculated_checksum) {
                    int received_ttl = ip_hdr->ttl;
                    emit errorResponse(QString("Received ICMP reply from %1: SEQ=%2 time=%3 TTL=%4")
                                      .arg(inet_ntoa(reply_addr.sin_addr))
                                      .arg(recv_sequence)
                                      .arg((double)delay / 1000.0)
                                      .arg(received_ttl));
                    emit icmpResponseTime(recv_sequence, (double)delay / 1000.0, " ");
                    break;
                }else{
                    emit errorResponse(QString("Received ICMP echo reply from %1 with invalid checksum")
                                                          .arg(inet_ntoa(reply_addr.sin_addr)));
                }
            }else{
                /* No data available yet, try to receive again. */
                continue;
            }
        }
next:
        if (delay < m_interval) {
//            qDebug() << "delay:" << delay ;
            usleep(REQUEST_INTERVAL - delay);
        }
//        QThread::sleep(1);  // Adjust the delay as needed
        sequence++;
    }

    close_socket(sockfd);
//    emit icmpResponse("ICMP sending stopped.");
    emit finished(m_idx);
}

void IcmpWrapper::onStarted()
{
//    printf("Pinging %s (%s)\n", hostname, addr_str);
//    fflush(stdout);
    qDebug() << "Pinging " << m_target;

}

void IcmpWrapper::onStoped(int idx)
{
    Q_UNUSED(idx)
    qDebug() << "onStoped: " << m_results;
}

void IcmpWrapper::onResponseTime(uint16_t seq, double responseTime, const char *checksum)
{
    if (responseTime>=0){
        qDebug() << QString("Reply from %1: seq=%2, time=%3, %4").arg(m_target)
                   .arg(seq).arg(responseTime).arg(checksum);
    }else {
        qDebug() << QString("Request timed out: seq=%1").arg(seq);
    }
    m_results.insert(seq, responseTime);
}

int IcmpWrapper::pingHost(QString &shostname, uint16_t id)
{
    //BUGS: this will cause APP crash on finished!!
    int error=0;
    struct addrinfo *addrinfo_list = NULL;
    struct addrinfo *addrinfo;
    std::string str=shostname.toStdString();
    const char *hostname = str.c_str();
    socket_t sockfd = -1;
    int icmp_payload_size=ICMP_PAYLOAD_SIZE;
    struct sockaddr_storage addr;
    socklen_t dst_addr_len;

    uint16_t seq;
    uint64_t start_time;
    uint64_t delay;
//    int max_num=0; // number of echo request, TODO: handle number over 65535?

#ifdef _WIN32
    init_winsock_lib();
#endif

    if (ip_version == IP_V4 || ip_version == IP_VERSION_ANY) {
        struct addrinfo hints ;//= {0};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_RAW;
        hints.ai_protocol = IPPROTO_ICMP;
        error = getaddrinfo(hostname,
                            NULL,
                            &hints,
                            &addrinfo_list);
    }
    if (ip_version == IP_V6
        || (ip_version == IP_VERSION_ANY && error != 0)) {
        struct addrinfo hints ;//= {0};
        hints.ai_family = AF_INET6;
        hints.ai_socktype = SOCK_RAW;
        hints.ai_protocol = IPPROTO_ICMPV6;
        error = getaddrinfo(hostname,
                            NULL,
                            &hints,
                            &addrinfo_list);
    }
    if (error != 0) {
        if (error == EAI_SYSTEM){
//            fprintf(stderr, "getaddrinfo: %s\n", strerror(errno));
            qDebug() << "getaddrinfo: " << strerror(errno);
        }else{
//            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(error));
            qDebug() << "getaddrinfo: " << gai_strerror(errno);
        }
//        goto exit_error;
        return EXIT_FAILURE;
    }

    for (addrinfo = addrinfo_list;
        addrinfo != NULL;
        addrinfo = addrinfo->ai_next) {
        sockfd = socket(addrinfo->ai_family,
                        addrinfo->ai_socktype,
                        addrinfo->ai_protocol);
        if (sockfd >= 0) {
            break;
        }
        // QCoreApplication::processEvents(QEventLoop::AllEvents);
    }
    if ((int)sockfd < 0) {
        psockerror("socket");
        qDebug() << "init socket error:";
//        goto exit_error;
        return EXIT_FAILURE;
    }
    // char icmp_payload[icmp_payload_size];
    // Allocate space for the ICMP payload buffer
    char *icmp_payload = (char *)malloc(icmp_payload_size);
    // Fill the ICMP payload buffer with some data (if needed)
    // For example, you might fill it with zeros or some specific data
    memset(icmp_payload, 255, icmp_payload_size);

    memcpy(&addr, addrinfo->ai_addr, addrinfo->ai_addrlen);
    dst_addr_len = (socklen_t)addrinfo->ai_addrlen;

    freeaddrinfo(addrinfo_list);
    addrinfo = NULL;
    addrinfo_list = NULL;

#ifdef _WIN32
    init_winsock_extensions(sockfd);
#endif
    /*
     * Switch the socket to non-blocking I/O mode. This allows us to implement
     * the timeout feature.
     */
#ifdef _WIN32
    {
        u_long opt_value = 1;
        if (ioctlsocket(sockfd, FIONBIO, &opt_value) != 0) {
            psockerror("ioctlsocket");
            goto exit_error;
        }
    }
#else /* _WIN32 */
    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) == -1) {
        psockerror("fcntl");
        qDebug() << "set socket fcntl error:";
        goto exit_error;
    }
#endif /* !_WIN32 */

    if (addr.ss_family == AF_INET6) {
        /*
         * This allows us to receive IPv6 packet headers in incoming messages.
         */
        int opt_value = 1;
        error = setsockopt(sockfd,
                           IPPROTO_IPV6,
#if defined _WIN32 || defined __CYGWIN__
                           IPV6_PKTINFO,
#else
                           IPV6_RECVPKTINFO,
#endif
                           (char *)&opt_value,
                           sizeof(opt_value));
        if (error != 0) {
            psockerror("setsockopt");
            qDebug() << "AF_INET6 setsockopt error:";
            goto exit_error;
        }
    }

    /*
     * As opening raw sockets usually requires superuser privileges, we should
     * drop them as soon as possible for security reasons.
     */
#if !defined _WIN32
    /* Note: group ID must be set before user ID! */
    if (setgid(getgid() != 0)) {
        perror("setgid");
        qDebug() << "setgid error:";
        goto exit_error;
    }
    if (setuid(getuid()) != 0) {
        perror("setuid");
        qDebug() << "setuid error:";
        goto exit_error;
    }
#endif

    /*
     * Convert the destination IP-address to a string.
     */
    inet_ntop(addr.ss_family,
              addr.ss_family == AF_INET6
                  ? (void *)&((struct sockaddr_in6 *)&addr)->sin6_addr
                  : (void *)&((struct sockaddr_in *)&addr)->sin_addr,
              addr_str,
              sizeof(addr_str));

    emit started();
    //    printf("Pinging %s (%s)\n", hostname, addr_str);
    //    fflush(stdout);
    for (seq = 0; ; seq++) {
        if (m_count>0){
            if (seq>=m_count){
                break;
            }
        }
        // QCoreApplication::processEvents(QEventLoop::AllEvents);

        struct icmp request;

        request.icmp_type =
                addr.ss_family == AF_INET6 ? ICMP6_ECHO : ICMP_ECHO;
        request.icmp_code = 0;
        request.icmp_cksum = 0;
        request.icmp_id = htons(id);
        request.icmp_seq = htons(seq); // only accept 65535
#if !defined _WIN32
    //TODO: mingw64 how to add payload?
        // Copy the ICMP payload into the request packet
        memcpy(request.icmp_data, icmp_payload, icmp_payload_size);
#endif
        if (addr.ss_family == AF_INET6) {
            /*
             * Checksum is calculated from the ICMPv6 packet prepended
             * with an IPv6 "pseudo-header".
             *
             * https://tools.ietf.org/html/rfc2463#section-2.3
             * https://tools.ietf.org/html/rfc2460#section-8.1
             */
            struct icmp6_packet request_packet ;//= {0};

            request_packet.ip6_hdr.src = in6addr_loopback;
            request_packet.ip6_hdr.dst =
                ((struct sockaddr_in6 *)&addr)->sin6_addr;
            request_packet.ip6_hdr.plen = htons((uint16_t)ICMP_HEADER_LENGTH);
            request_packet.ip6_hdr.nxt = IPPROTO_ICMPV6;
            request_packet.icmp = request;

            request.icmp_cksum = compute_checksum((char *)&request_packet,
                                                   sizeof(request_packet));
            // request->checksum = compute_checksum((char *)&request_packet,
            //                                       sizeof(request_packet));
        } else {
            request.icmp_cksum = compute_checksum((char *)&request,
                                                  sizeof(request)+ icmp_payload_size);
            // request->checksum = compute_checksum((char *)&request,
            //                                       sizeof(request)+ icmp_payload_size);
        }

        // if (srcaddr != NULL) {
        //     // Initialize sockaddr_in structure
        //     int local_port = 12345; // Example port number
        //     struct sockaddr_in local_addr;
        //     memset(&local_addr, 0, sizeof(local_addr));
        //     local_addr.sin_family = AF_INET;
        //     local_addr.sin_addr.s_addr = inet_addr(srcaddr);
        //     local_addr.sin_port = htons(local_port);
        //     // Bind the socket to a specific local address (source address)
        //     if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0) {
        //         perror("bind");
        //         // Error handling if binding fails
        //         goto exit_error;
        //     }
        // }
//        qDebug() << "sendto:" << QString::number(seq);
        error = (int)sendto(sockfd,
                            (char *)&request,
                            sizeof(request)+ icmp_payload_size,
                            0,
                            (struct sockaddr *)&addr,
                            (int)dst_addr_len);
        if (error < 0) {
            psockerror("sendto");
            qDebug() << "sendto error:";
            goto exit_error;
        }

        start_time = utime();

        for (;;) {
            // QCoreApplication::processEvents(QEventLoop::AllEvents);
            char msg_buf[MESSAGE_BUFFER_SIZE];
            char packet_info_buf[MESSAGE_BUFFER_SIZE];
            struct in6_addr msg_addr = {0};
#ifdef _WIN32
            WSABUF msg_buf_struct = {
                sizeof(msg_buf),
                msg_buf
            };
            WSAMSG msg = {
                NULL,
                0,
                &msg_buf_struct,
                1,
                {sizeof(packet_info_buf), packet_info_buf},
                0
            };
            DWORD msg_len = 0;
#else /* _WIN32 */
            struct iovec msg_buf_struct = {
                msg_buf,
                sizeof(msg_buf)
            };
            struct msghdr msg = {
                NULL,
                0,
                &msg_buf_struct,
                1,
                packet_info_buf,
                sizeof(packet_info_buf),
                0
            };
            size_t msg_len;
#endif /* !_WIN32 */
            cmsghdr_t *cmsg;
            size_t ip_hdr_len;
            struct icmp *reply;
            int reply_id;
            int reply_seq;
            uint16_t reply_checksum;
            uint16_t checksum;

#ifdef _WIN32
            error = WSARecvMsg(sockfd, &msg, &msg_len, NULL, NULL);
#else
            error = (int)recvmsg(sockfd, &msg, 0);
#endif

            delay = utime() - start_time;

            if (error < 0) {
#ifdef _WIN32
                if (WSAGetLastError() == WSAEWOULDBLOCK) {
#else
                if (errno == EAGAIN) {
#endif
                    if (delay > m_timeout) {
                        if (showtimestemp){
                            current_time(timestempformat);
                        }
                        //timeout
                        emit icmpResponseTime(seq, -1);
                        goto next;
                    } else {
                        /* No data available yet, try to receive again. */
                        continue;
                    }
                } else {
                    psockerror("recvmsg");
                    qDebug() << "recvmsg error:";
                    goto next;
                }
            }

#ifndef _WIN32
            msg_len = error;
#endif

            if (addr.ss_family == AF_INET6) {
                /*
                 * The IP header is not included in the message, msg_buf points
                 * directly to the ICMP data.
                 */
                ip_hdr_len = 0;

                /*
                 * Extract the destination address from IPv6 packet info. This
                 * will be used to compute the checksum later.
                 */
                for (
                    cmsg = CMSG_FIRSTHDR(&msg);
                    cmsg != NULL;
                    cmsg = CMSG_NXTHDR(&msg, cmsg))
                {
                    if (cmsg->cmsg_level == IPPROTO_IPV6
                        && cmsg->cmsg_type == IPV6_PKTINFO) {
                        struct in6_pktinfo *pktinfo = (struct in6_pktinfo *)CMSG_DATA(cmsg);
                        memcpy(&msg_addr,
                               &pktinfo->ipi6_addr,
                               sizeof(struct in6_addr));
                    }
                }
            } else {
                /*
                 * For IPv4, we must take the length of the IP header into
                 * account.
                 *
                 * Header length is stored in the lower 4 bits of the VHL field
                 * (VHL = Version + Header Length).
                 */
                ip_hdr_len = ((*(uint8_t *)msg_buf) & 0x0F) * 4;
            }

            reply = (struct icmp *)(msg_buf + ip_hdr_len);
            reply_id = ntohs(reply->icmp_id);
            reply_seq = ntohs(reply->icmp_seq);

            /*
             * Verify that this is indeed an echo reply packet.
             */
            if (!(addr.ss_family == AF_INET
                  && reply->icmp_type == ICMP_ECHO_REPLY)
                && !(addr.ss_family == AF_INET6
                     && reply->icmp_type == ICMP6_ECHO_REPLY)) {
                continue;
            }

            /*
             * Verify the ID and sequence number to make sure that the reply
             * is associated with the current request.
             */
            if (reply_id != id || reply_seq != seq) {
                continue;
            }

            reply_checksum = reply->icmp_cksum;
            reply->icmp_cksum = 0;

            /*
             * Verify the checksum.
             */
            if (addr.ss_family == AF_INET6) {
                size_t size = sizeof(struct ip6_pseudo_hdr) + msg_len;
                struct icmp6_packet *reply_packet = (struct icmp6_packet *)calloc(1, size);

                if (reply_packet == NULL) {
                    psockerror("malloc");
                    qDebug() << "malloc error:";
                    goto exit_error;
                }

                memcpy(&reply_packet->ip6_hdr.src,
                       &((struct sockaddr_in6 *)&addr)->sin6_addr,
                       sizeof(struct in6_addr));
                reply_packet->ip6_hdr.dst = msg_addr;
                reply_packet->ip6_hdr.plen = htons((uint16_t)msg_len);
                reply_packet->ip6_hdr.nxt = IPPROTO_ICMPV6;
                memcpy(&reply_packet->icmp,
                       msg_buf + ip_hdr_len,
                       msg_len - ip_hdr_len);

                checksum = compute_checksum((char *)reply_packet, size);
            } else {
                checksum = compute_checksum(msg_buf + ip_hdr_len,
                                            msg_len - ip_hdr_len);
            }
            if (showtimestemp){
                current_time(timestempformat);
            }
//            printf("Reply from %s: seq=%d, time=%.3f ms%s\n",
//                   addr_str,
//                   seq,
//                   (double)delay / 1000.0,
//                   reply_checksum != checksum ? " (bad checksum)" : "");
//            fflush(stdout);
            emit icmpResponseTime(seq, (double)delay / 1000.0, reply_checksum != checksum ? " (bad checksum)":"");
            break;
        }

next:
        if (delay < REQUEST_INTERVAL) {
//            qDebug() << "delay:" << delay ;
            usleep(REQUEST_INTERVAL - delay);
        }
    }
    qDebug() << "close_socket" ;
    close_socket(sockfd);
    qDebug() << "return" ;
    return EXIT_SUCCESS;

exit_error:

    if (addrinfo_list != NULL) {
        freeaddrinfo(addrinfo_list);
    }

    close_socket(sockfd);
    return EXIT_FAILURE;
}

void IcmpWrapper::getTTLs()
{
    qDebug() << "getTTLs" << m_results;
}

QByteArray IcmpWrapper::createIcmpPacket(int packet_id, int sequence) {
    struct icmphdr icmp_hdr;
    QByteArray packet;
if (ip_version == IP_V4){
    icmp_hdr.type = ICMP_ECHO;
}else if(ip_version == IP_V6){
    icmp_hdr.type = ICMP6_ECHO;
}
    icmp_hdr.code = 0;
    icmp_hdr.un.echo.id = htons(packet_id);
    icmp_hdr.un.echo.sequence = htons(sequence);
    icmp_hdr.checksum = 0;

    packet.append(reinterpret_cast<const char*>(&icmp_hdr), sizeof(icmp_hdr));
    if (m_packetsize>0){
        int appendsize = m_packetsize - 14 - 20 - 8;  // MAC: 14, IP:20, ICMP:8
        if (appendsize>0){
            qDebug() << "TODO: append size:" << appendsize;
            QString str(appendsize, 'A');
            packet.append(str.toStdString().c_str(), appendsize);
        }
//    packet.append("QTICMP", 6);  // Add some data, TODO: specify size
    }


    icmp_hdr.checksum = calculateChecksum(packet.data(), packet.size());

    memcpy(packet.data(), &icmp_hdr, sizeof(icmp_hdr));  // Update the checksum in the packet

    return packet;
}

uint16_t IcmpWrapper::calculateChecksum(const char* data, int len) {
    uint32_t sum = 0;
    uint16_t* ptr = (uint16_t*)data;

    for (int i = 0; i < len / 2; i++) {
        sum += *ptr++;
    }

    if (len & 1) {
        sum += *(uint8_t*)ptr;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
}

unsigned short IcmpWrapper::calculateChecksum(void *b, int len) {
    unsigned short *buf = (unsigned short *)b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

void IcmpWrapper::current_time(const char *timestempformat) {
    Q_UNUSED(timestempformat) // TODO: custom timestemp format
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%Y%m%d_%H:%M:%S ", timeinfo);
    //strftime(buffer, sizeof(buffer), &timestempformat, timeinfo);
    printf("%s", buffer);
}


