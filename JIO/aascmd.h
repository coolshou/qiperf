#ifndef AASCMD_H
#define AASCMD_H

#define JIO_GET_SENSORS "GET_SENSORS"  // ask to get sensor data by sensors_call_so
#define JIO_SENSORS_DATA "SENSORS_DATA"  // return sensor data

#define JIO_GET_GPS "GET_GPS"  // ask to get sensor data by gps_call_so
#define JIO_GPS_DATA "GPS_DATA"  // return gps data

#define JIO_GET_AP_INFO "GET_AP_INFO"
#define JIO_AP_INFO "AP_INFO"

#define JIO_GET_BEAMFACTOR "GET_BEAMFACTOR" // get BEAMFACTOR/BEAMType
#define JIO_SET_BEAMFACTOR "21" // set BEAMFACTOR/BEAMType

#define JIO_GET_BEAMDIRECTION "GET_BEAMDIRECTION" // get Beam table (Direction) ID
#define JIO_SET_BEAMDIRECTION "31" // set Beam table (Direction) ID

#define JIO_GET_RSSI "GET_RSSI" // get RSSI
#define JIO_RSSI_DATA "41" // RSSI data

#define JIO_GET_MCS "GET_MCS" // get MCS
#define JIO_MCS_DATA "51" // MCS data

#define JIO_GET_PRODUCTTYPE "GET_PRODUCTTYPE"  // CM7, AM7
#define JIO_GET_AIP1VENDOR "GET_AIP1VENDOR"
#define JIO_GET_AIP2VENDOR "GET_AIP2VENDOR"

// output will be in dmesg, need parser it
// logread -f

// CM7 MCS AP "iwpriv rax0 show stainfo"
// CM7 MCS Client "mwctl apclix0 show stainfo=all"

// getmfg opMode: STA, AP
// Value: STA
#define JIO_GET_OPMODE "GET_OPMODE" // get OPMODE
#define JIO_OPMODE_DATA "61" // OPMODE data

// need parser logread -f output
// AP show STA info
//iwpriv rax0 show stainfo
// STA show AP info
//mwctl apclix0 show stainfo=all

// set before run throughput
// AM7: mfc set D2D_iperf_better_7988A
// CM7: mfc set D2D_iperf_better_7988D

#define JIO_SET_AM7_IPERF_BETTER "mfc set D2D_iperf_better_7988A"
#define JIO_SET_CM7_IPERF_BETTER "mfc set D2D_iperf_better_7988D"



#endif // AASCMD_H
