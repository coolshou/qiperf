#ifndef COMM_H
#define COMM_H

#define QIPERF_DOMAIN "alphanetworks.com"
#define QIPERF_ORG "alphanetworks"
#define QIPERFD_NAME  "qiperfd"
#define QIPERFTRAY_NAME  "qiperftray"
#define QIPERFD_PORT 47014
#define QIPERFD_BPORT 47015  //broadcast
#define QIPERFD_WSPORT 47016  //websocket port
#define RPC_PORT 57025

#define QIPERFC_NAME  "qiperfconsole"

#define CMD_OK     "OK"
#define CMD_FAIL   "FAIL"
#define CMD_ARGS   "ARGS"   // send args: eq: -k: kill qiperfd
#define CMD_STATUS "STATUS"
#define CMD_IFNAMES "IFNAMES"  //Get all interfaces
#define CMD_SET_IFNAME "SET_IFNAME"  //Get all interfaces
#define CMD_RUNNING "RUNNING"  //get any iperf running status
//iperf control
#define CMD_IPERF_START  "IPERF_START"
#define CMD_IPERF_STOP   "IPERF_STOP"
#define CMD_IPERF_ADD "IPERF_ADD"

enum class IPERF_VER {
    V1=0,  //1.7.0
    V2=1,  //2.0.14
    V21=2, //2.1.9
    V3=3   //3.14
};

//TEST
#define TEST_JSONRPC 0
#define TEST_PLOT_DATA 0
#define TEST_WS 1
#define USE_JSONRPC 0


#endif // COMM_H
