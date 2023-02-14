#ifndef COMM_H
#define COMM_H

#define QIPERF_DOMAIN "alphanetworks.com"
#define QIPERF_ORG "alphanetworks"
#define QIPERFD_NAME  "qiperfd"
#define QIPERFTRAY_NAME  "qiperftray"
#define QIPERFD_PORT 47014
#define RPC_PORT 57025

#define QIPERFC_NAME  "qiperfconsole"

#define CMD_OK     "OK"
#define CMD_FAIL   "FAIL"

#define CMD_STATUS "status"
#define CMD_IFNAMES "IFNAMES"  //Get all interfaces
//iperf control
#define CMD_IPERF_START  "IPERF_START"
#define CMD_IPERF_STOP   "IPERF_STOP"
#define CMD_IPERF_ADD "IPERF_ADD"

#endif // COMM_H
