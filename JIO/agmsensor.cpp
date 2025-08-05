#include "agmsensor.h"

AGMSensor::AGMSensor(QObject *parent)
    : QObject{parent}
{}

QString AGMSensor::sendQuery()
{
    QString cmd = "sensors_call_so";
    return cmd;
}

/*
sensor Data Received by sensor.so:
sensor Data Received:
in_acc[0]: 0.003155
in_acc[1]: 0.045063
in_acc[2]: 0.995394
in_gyro[0]: 0.131250
in_gyro[1]: -0.612500
in_gyro[2]: 0.192500
in_mag[0]: 0.519000
in_mag[1]: -0.182000
in_mag[2]: -1.232000
out_rotation[0]: 285.543610
out_rotation[1]: -89.847168
out_rotation[2]: 2.657148
out_quaternion[0]: 0.572051
out_quaternion[1]: 0.414036
out_quaternion[2]: 0.441285
out_quaternion[3]: 0.553715
out_gravity[0]: -0.046359
out_gravity[1]: -0.998921
out_gravity[2]: -0.002665
out_linear_acceleration[0]: -0.001296
out_linear_acceleration[1]: -0.003528
out_linear_acceleration[2]: 0.000490
out_heading: 285.543610
out_headingErr: 0.017453
Version_information: ST MotionFX v2.7.1
SENSOR_Chipset_Information: ism330dhcx and lis2mdl
ODM_Information: AlphaNetworks
Calibration_status_of_the_Sensors: 3
Qulity is very good.
Sensor mode: Polling

*/
void AGMSensor::parserData(QString data)
{   int status=0;
    double heading=0.0;
    double headingerror=0.0;
    double pitch=0.0;
    QStringList ds;

    for (QString line: data.split("\n")){
        if (line.contains("Calibration_status_of_the_Sensors:")){
            ds = line.split(":");
            if (ds.length()==2){
                status = ds[1].toInt();
            }
        }
        if (line.contains("out_heading:")){
            ds = line.split(":");
            if (ds.length()==2){
                heading = ds[1].toDouble();
            }
        }
        if (line.contains("out_headingErr:")){
            ds = line.split(":");
            if (ds.length()==2){
                headingerror = ds[1].toDouble();
            }
        }
        if (line.contains("out_rotation[1]:")){
            ds = line.split(":");
            if (ds.length()==2){
                pitch = ds[1].toDouble();
            }
        }
    }
    emit updateData(status, heading, headingerror, pitch);
}

