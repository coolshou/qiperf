#/bin/bash

# sudo apt install sshpass

# build deb package
DOBUILD=0
CODENAME=`grep '^VERSION_CODENAME' /etc/os-release | cut -d= -f2`
WINVERSION=`grep '#define QIPERFD_VERSION' src/versions.h | cut -d\"  -f2`
VERSION=${WINVERSION}-1


declare -a DESTFILES=()
DESTFILES+=(qiperfd_${VERSION}${CODENAME}_amd64.deb)
#DESTFILES+=(qiperftray_${VERSION}${CODENAME}_amd64.deb)

declare -a WDESTFILES=()
WDESTFILES+=(qiperf-setup-${WINVERSION}.exe)

# ================================================
UPDATE_LINUX=1
declare -a IPS=()
#IPS+=("192.168.70.11")
IPS+=("192.168.70.13")
IPS+=("192.168.70.14")
#IPS+=("192.168.70.12")
#IPS+=("192.168.70.23")
#IPS+=("192.168.70.24")
#IPS+=("192.168.70.135")
#IPS+=("192.168.70.31")
#IPS+=("192.168.70.32")
#IPS+=("192.168.70.154")
#IPS+=("192.168.70.162")
#IPS+=("192.168.70.147") # not test user

#windows remote
declare -a WIPS=()
#WIPS+=("192.168.70.21")
#WIPS+=("192.168.70.11")

#
USERNAME=test
PASSWORD=123456
#RvR
UPDATE_RVR=0
RVRIP="172.31.117.119"
declare -a RVRPORT=()
RVRPORT+=(60020)
RVRPORT+=(60010)

# Room6 - TR398 PCs
DOREMOTE=0
DOREMOTEWIN=0 #; remote is windows
DOREMOTEIP="172.31.117.120"

if [ $DOREMOTEWIN -eq 1 ]; then
  TARGET=test@${DOREMOTEIP}:/D:/
  DESTFILES=${WDESTFILES}
  INSTCMD="/D:/${WDESTFILES} -s"
else
  TARGET=test@${DOREMOTEIP}
  INSTCMD="sudo dpkg -i /home/test/${DESTFILES}"
fi

declare -a PORTS=()
PORTS+=(55901)
PORTS+=(55902)
#PORTS+=(55903)
#PORTS+=(55904)
#PORTS+=(55905)
PORTS+=(55906)
#PORTS+=(55908)
#PORTS+=(55911) # LAN
#PORTS+=(55912) # LAN2
#PORTS+=(55920)


if [ "x$DOBUILD" == "x1" ]; then
    # build
    build_deb.sh
fi

if [ "x$?" == "x0" ]; then
    if [ $UPDATE_LINUX -eq 1 ]; then
        for IP in "${IPS[@]}"
        do
            for DESTFILE in "${DESTFILES[@]}"
            do
                echo "================================================================================"
                echo "===== ssh ${USERNAME}@${IP} rm /home/test/${DESTFILE}"
                ssh ${USERNAME}@${IP} rm /home/test/${DESTFILE} > /dev/null
                echo "===== scp ${DESTFILE} ${USERNAME}@${IP}:/home/test/${DESTFILE}"
                scp ${DESTFILE} ${USERNAME}@${IP}:/home/test/${DESTFILE} > /dev/null 2>&1
                if [ $? == 0 ]; then
                    echo "===== ssh ${USERNAME}@${IP} sshpass -p '123456' sudo dpkg -i /home/test/${DESTFILE}"
                    ssh ${USERNAME}@${IP} sshpass -p '123456' sudo dpkg -i /home/test/${DESTFILE} > /dev/null 2>&1
                    if [ $? != 0 ]; then
                       echo "***** Fail install /home/test/${DESTFILE} on ${USERNAME}@${IP} *****"
                    fi
                else
                    echo "**** upload ${DESTFILE} Fail"
                fi
            done
        done
    fi
    if [ $DOREMOTE -eq 1 ]; then
        #scp ../${DESTFILE} test@192.168.70.31:/home/test/${DESTFILE}
        # Room6 TR398
        for PORT in "${PORTS[@]}"
        do
            for DESTFILE in "${DESTFILES[@]}"
            do
                echo "===== scp -P $PORT ${DESTFILE}  ${TARGET}/home/test/${DESTFILE}"
                scp -P $PORT ${DESTFILE} ${TARGET}/home/test/${DESTFILE}
                if [ $? == 0 ]; then
                    echo "===== ssh -p $PORT ${USERNAME}@${DOREMOTEIP} ${INSTCMD}"
                    ssh -p $PORT ${USERNAME}@${DOREMOTEIP} ${INSTCMD}
                else
                    echo "upload ${DESTFILE} Fail"
                fi
            done
        done
    fi
    # window system
    for IP in "${WIPS[@]}"
    do
        for WINSETUP in "${WDESTFILES[@]}"
        do
            echo "===== scp ${WINSETUP} ${USERNAME}@${IP}:D:\\${WINSETUP}"
            scp ${WINSETUP} ${USERNAME}@${IP}:D:\\${WINSETUP}
            if [ $? == 0 ]; then
                echo "===== ssh ${USERNAME}@${IP} D:\\${WINSETUP} /S"
                ssh ${USERNAME}@${IP} D:\\${WINSETUP} /S
            else
                echo "upload ${WINSETUP} Fail"
            fi
        done
    done
    if [ $UPDATE_RVR -eq 1 ]; then
        for PORT in "${RVRPORT[@]}"
        do
            for WINSETUP in "${WDESTFILES[@]}"
            do
                echo "===== scp -P ${PORT} ${WINSETUP} ${USERNAME}@${RVRIP}:D:\\${WINSETUP}"
                scp -P ${PORT} ${WINSETUP} ${USERNAME}@${RVRIP}:D:\\${WINSETUP}
                if [ $? == 0 ]; then
                    echo "===== ssh -P ${PORT} ${USERNAME}@${RVRIP} D:\\${WINSETUP} /S"
                    ssh -P ${PORT} ${USERNAME}@${RVRIP} D:\\${WINSETUP} /S
                else
                    echo "upload ${WINSETUP} Fail"
                fi
            done
        done
    fi
fi
