#ifndef AGMSENSOR_H
#define AGMSENSOR_H

#include <QObject>

/*
 ISM330DHCX: 3D accelerometer and 3D gyroscope
 LIS2MDL:    3-axis magnetometer

helper class to parser about info from device

> sensors_call_so
[Tue Jul 22 16:45:44.208 2025] sensor Data Received by sensor.so:
[Tue Jul 22 16:45:44.216 2025] sensor Data Received:
[Tue Jul 22 16:45:44.216 2025] in_acc[0]: 0.006283
[Tue Jul 22 16:45:44.220 2025] in_acc[1]: 0.954345
[Tue Jul 22 16:45:44.220 2025] in_acc[2]: 0.287188
[Tue Jul 22 16:45:44.231 2025] in_gyro[0]: 0.096250
[Tue Jul 22 16:45:44.231 2025] in_gyro[1]: -0.533750
[Tue Jul 22 16:45:44.234 2025] in_gyro[2]: 0.201250
[Tue Jul 22 16:45:44.234 2025] in_mag[0]: -0.897000
[Tue Jul 22 16:45:44.234 2025] in_mag[1]: -0.312000
[Tue Jul 22 16:45:44.234 2025] in_mag[2]: 1.512000
[Tue Jul 22 16:45:44.235 2025] out_rotation[0]: 167.835693
[Tue Jul 22 16:45:44.235 2025] out_rotation[1]: -88.403496
[Tue Jul 22 16:45:44.240 2025] out_rotation[2]: 73.279076
[Tue Jul 22 16:45:44.244 2025] out_quaternion[0]: 0.366142
[Tue Jul 22 16:45:44.244 2025] out_quaternion[1]: 0.601608
[Tue Jul 22 16:45:44.251 2025] out_quaternion[2]: 0.527914
[Tue Jul 22 16:45:44.251 2025] out_quaternion[3]: -0.474673
[Tue Jul 22 16:45:44.263 2025] out_gravity[0]: -0.957717
[Tue Jul 22 16:45:44.263 2025] out_gravity[1]: -0.287598
[Tue Jul 22 16:45:44.263 2025] out_gravity[2]: -0.008016
[Tue Jul 22 16:45:44.267 2025] out_linear_acceleration[0]: -0.003372
[Tue Jul 22 16:45:44.268 2025] out_linear_acceleration[1]: -0.000410
[Tue Jul 22 16:45:44.268 2025] out_linear_acceleration[2]: -0.001733
[Tue Jul 22 16:45:44.273 2025] out_heading: 169.502640
[Tue Jul 22 16:45:44.273 2025] out_headingErr: 0.017453
[Tue Jul 22 16:45:44.273 2025] Version_information: ST MotionFX v2.7.1
[Tue Jul 22 16:45:44.278 2025] SENSOR_Chipset_Information: ism330dhcx and lis2mdl
[Tue Jul 22 16:45:44.283 2025] ODM_Information: AlphaNetworks
[Tue Jul 22 16:45:44.288 2025] Calibration_status_of_the_Sensors: 0
[Tue Jul 22 16:45:44.288 2025] Please refence to the um2220, page 7:
[Tue Jul 22 16:45:44.292 2025] After calibration routine initialization:
[Tue Jul 22 16:45:44.298 2025] Slowly rotate the device in a figure 8 pattern in space.
[Tue Jul 22 16:45:44.302 2025] While performing this movement, keep the device clear of other magnetic objects such as cell phones, computers and other steel objects.

# Clear Calibration Value: st_motion c m

# when Calibration finish:
==> Calibration_status_of_the_Sensors = 3

out_rotation[0]: Yaw   (偏航角) - 指北方向
out_rotation[1]: Pitch (俯仰角) - 上下傾斜
out_rotation[2]: Roll  (翻轉角) - 左右傾斜

面朝上 out_rotation[1]=-90
面朝下 out_rotation[1]=+90

*/

class AGMSensor : public QObject
{
    Q_OBJECT
public:
    explicit AGMSensor(QObject *parent = nullptr);

signals:
private:
    double mYaw; //degree
    double mPitch; //degree
    double mRoll; //degree
    double mHeading; // final heading
    double mHeadingErr; //
    double mHeadingErrLimit;
    int mCalibrationStatus;
    QString mVersion; // store SENSOR Version_information
};

#endif // AGMSENSOR_H
