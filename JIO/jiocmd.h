#ifndef JIOCMD_H
#define JIOCMD_H

#define JIO_GET_SENSORS "0"  // ask to get sensor data by sensors_call_so
#define JIO_SENSORS_DATA "1"  // return sensor data

#define JIO_GET_GPS "10"  // ask to get sensor data by gps_call_so
#define JIO_GPS_DATA "11"  // return gps data

#define JIO_GET_BEAMFACTOR "20" // get BEAMFACTOR/BEAMType
#define JIO_SET_BEAMFACTOR "21" // set BEAMFACTOR/BEAMType

#define JIO_GET_BEAMDIRECTION "30" // get Beam table (Direction) ID
#define JIO_SET_BEAMDIRECTION "31" // set Beam table (Direction) ID

#define JIO_GET_RSSI "40" // get RSSI
#define JIO_RSSI_DATA "41" // RSSI data

#define JIO_GET_MCS "50" // get MCS
#define JIO_MCS_DATA "51" // MCS data

// getmfg opMode: STA, AP
// Value: STA
#define JIO_GET_OPMODE "60" // get OPMODE
#define JIO_OPMODE_DATA "61" // OPMODE data

//iwpriv rax0 show stainfo
//mwctl apclix0 show stainfo=all

// set before run throughput
// AM7: mfc set D2D_iperf_better_7988A
// CM7: mfc set D2D_iperf_better_7988D

#define JIO_SET_AM7_IPERF_BETTER "mfc set D2D_iperf_better_7988A"
#define JIO_SET_CM7_IPERF_BETTER "mfc set D2D_iperf_better_7988D"



#endif // JIOCMD_H
