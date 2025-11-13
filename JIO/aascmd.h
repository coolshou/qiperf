#ifndef AASCMD_H
#define AASCMD_H

#define AAS_GET_SENSORS "GET_SENSORS"  // ask to get sensor data by sensors_call_so
#define AAS_SENSORS_DATA "SENSORS_DATA"  // return sensor data

#define AAS_GET_GPS "GET_GPS"  // ask to get sensor data by gps_call_so
#define AAS_GPS_DATA "GPS_DATA"  // return gps data

#define AAS_GET_AP_INFO "GET_AP_INFO"
#define AAS_AP_INFO "AP_INFO"

#define AAS_GET_BEAMFACTOR "GET_BEAMFACTOR" // get BEAMFACTOR/BEAMType
#define AAS_SET_BEAMFACTOR "21" // set BEAMFACTOR/BEAMType

#define AAS_GET_BEAMDIRECTION "GET_BEAMDIRECTION" // get Beam table (Direction) ID
#define AAS_SET_BEAMDIRECTION "31" // set Beam table (Direction) ID

#define AAS_GET_RSSI "GET_RSSI" // get RSSI
#define AAS_RSSI_DATA "41" // RSSI data

#define AAS_GET_MCS "GET_MCS" // get MCS
#define AAS_MCS_DATA "51" // MCS data

#define AAS_GET_PRODUCTTYPE "GET_PRODUCTTYPE"  // CM7, AM7
#define AAS_GET_AIP1VENDOR "GET_AIP1VENDOR"
#define AAS_GET_AIP2VENDOR "GET_AIP2VENDOR"

// output will be in dmesg, need parser it
// logread -f

// CM7 MCS AP "iwpriv rax0 show stainfo"
// CM7 MCS Client "mwctl apclix0 show stainfo=all"

// getmfg opMode: STA, AP
// Value: STA
#define AAS_GET_OPMODE "GET_OPMODE" // get OPMODE
#define AAS_OPMODE_DATA "61" // OPMODE data

// need parser logread -f output
// AP show STA info
//iwpriv rax0 show stainfo
// STA show AP info
//mwctl apclix0 show stainfo=all

// set before run throughput
// AM7: mfc set D2D_iperf_better_7988A
// CM7: mfc set D2D_iperf_better_7988D

#define AAS_SET_AP_IPERF_BETTER "mfc set D2D_iperf_better_7988A"
#define AAS_SET_STA_IPERF_BETTER "mfc set D2D_iperf_better_7988D"



#endif // AASCMD_H
