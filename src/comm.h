#ifndef COMM_H
#define COMM_H

#define QIPERF_DOMAIN "alphanetworks.com"
#define QIPERF_ORG "alphanetworks"

#define QIPERFC_NAME  "qiperfconsole"
#define QIPERF_NAME  "qiperf"
#define QIPERFD_NAME  "qiperfd"
#define QIPERFDLOG "QIPERFDLOG"

#define QIPERFTRAY_NAME  "qiperftray"

#define QIPERF_EXT "qip"
#define QIPERF_EXT_FILTER "qiperf (*.qip)"
#define ALL_EXT_FILTER "All (*.*)"
#define HTML_EXT "html"
#define HTML_EXT_FILTER "html (*.html)"

#define QIPERFD_PORT 47014
#define QIPERFD_BPORT 47015  //broadcast
#define QIPERFD_WSPORT 47016  //websocket port
#define QIPERFD_WSNAME "WS Server"
#define QIPERF_FILEPORT 47017 //FileServer Port
#define QIPERF_SERIALPORT 47020 // basic port use for serial data transfer
#define RPC_PORT 57025


#define CMD_OK     "OK"
#define CMD_FAIL   "FAIL"
#define CMD_ARGS   "ARGS"   // send args: eq: -k: kill qiperfd
#define CMD_STATUS "STATUS"

#define CMD_QIPERFD_START "QIPERFD_START" // ASK qiperfd start
#define INFO_QIPERFD_STARTED "QIPERFD_STARTED"  // inform qiperfd started
#define CMD_QIPERFD_STOP  "QIPERFD_STOP" // ASK qiperfd stop
#define INFO_QIPERFD_STOPED "QIPERFD_STOPED" // inform qiperfd stoped

#define CMD_QIPERFD_RESTART "QIPERFD_RESTART" // ASK qiperfd restart
#define CMD_IFNAMES "IFNAMES"  //Get all interfaces
#define CMD_SET_IFNAME "SET_IFNAME"  //Get all interfaces
#define CMD_RUNNING "RUNNING"  //get any iperf running status
#define CMD_GET_LOGFILENAME "GET_LOGFILENAME"  // get qiperfd log file path
#define CMD_ERROR "ERROR"  // show error message by qiperftray's balloon message
#define CMD_INFO "INFO"  // show info message by qiperftray's balloon message
#define CMD_IPERFVER "IPERFVER" // get iperf version

//iperf control
#define CMD_IPERF_START  "IPERF_START" // start iperf
#define CMD_IPERF_STARTED  "IPERF_STARTED" // iperf is running
#define CMD_IPERF_STOP   "IPERF_STOP"  // stop iperf
#define CMD_IPERF_STOPED   "IPERF_STOPED"  // iperf is stop
#define CMD_IPERF_ERRORED   "IPERF_ERRORED"  // iperf is error
#define CMD_IPERF_ADD    "IPERF_ADD"   // add iperf setting
#define CMD_IPERF_REG    "IPERF_REG" // reg iperf endpoint to report throughput result
#define CMD_IPERF_UNREG  "IPERF_UNREG" // reg iperf endpoint not to report throughput result
#define CMD_IPERF_DEL    "IPERF_DEL" // del iperf setting
#define CMD_IPERF_CLEAR    "IPERF_CLEAR" // clear all iperf setting
#define CMD_IPERF_TP_DATA  "IPERF_TP_DATA"  // report iperf throughput data
#define CMD_IPERF_TP_FILE  "IPERF_TP_FILE"  // iperf throughput data filename with full path
#define CMD_IPERF_GET_TP_FILE  "IPERF_GET_TP_FILE"  // get iperf throughput data file
// serial
#define CMD_SERIAL_ADD    "SERIAL_ADD"   // add SERIAL setting for rs232

// ping
#define CMD_PING        "PING"

#define MYTIMESTEMP "yyyy-MM-dd hh:mm:ss.zzz"
#define DATETIME_NOW_FORMAT "yyyy-MM-dd_hhmmss.zzz"

enum class IPERF_VER {
    V1=0,  //1.7.0
    V2=1,  //2.0.14
    V21=2, //2.1.9
    V3=3   //3.14
};

#define READ_BUFFER_SIZE_PATH "/proc/sys/net/core/rmem_max"
#define WRITE_BUFFER_SIZE_PATH "/proc/sys/net/core/wmem_max"

enum class BUFFER_SIZES {
    KB=1024,
    MB=1048576
};

// linux default 64
#define BYPASS_HOTSPOT_TTL 65
#define IPv4_TTL_PATH "/proc/sys/net/ipv4/ip_default_ttl"
#define IPv6_TTL_PATH "/proc/sys/net/ipv6/conf/all/hop_limit"

// set hop limit // Windows : default 128
//netsh interface ipv4 set global defaultcurhoplimit=65
//netsh interface ipv6 set global defaultcurhoplimit=65
// netsh interface ipv4 show global
// PowerShell prompt enter Get-Command -Module NetTCPIP
// show ttl value
// (Get-NetIPv4Protocol).DefaultHopLimit
// (Get-NetIPv6Protocol).DefaultHopLimit


//TEST
#define TEST_WS 1
#define TEST_ICMP 0

//DEBUG
#define DEBUG_EXPORT_HTML 0

#endif // COMM_H
