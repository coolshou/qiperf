#/bin/bash

# build deb package
DOBUILD=0

VERSION=0.7.11312.23-1
#VERSION=0.6.11312.04
declare -a DESTFILES=()
DESTFILES+=(qiperfd_${VERSION}_amd64.deb)
DESTFILES+=(qiperftray_${VERSION}_amd64.deb)

declare -a WDESTFILES=()
WDESTFILES+=(qiperf-setup-${VERSION}.exe)

# ================================================
UPDATE_LINUX=1
declare -a IPS=()
#IPS+=("192.168.70.11")
#IPS+=("192.168.70.13")
#IPS+=("192.168.70.12")
IPS+=("192.168.70.23")
IPS+=("192.168.70.24")
#IPS+=("192.168.70.135")
#IPS+=("192.168.70.31")
#IPS+=("192.168.70.154")
#IPS+=("192.168.70.162")
#IPS+=("192.168.70.147") # not test user

#windows remote
declare -a WIPS=()
#WIPS+=("192.168.70.21")
#WIPS+=("192.168.70.11")



#RvR
UPDATE_RVR=0
RVRIP="172.31.117.119"
declare -a RVRPORT=()
RVRPORT+=(60020)
RVRPORT+=(60010)

# Room6 - TR398 PCs
DOREMOTE=0
DOREMOTEIP="172.31.117.120"
declare -a PORTS=()
#PORTS+=(55901)
#PORTS+=(55902)
#PORTS+=(55903)
PORTS+=(55904)
PORTS+=(55905)
#PORTS+=(55906)
#PORTS+=(55908)
PORTS+=(55911) # LAN
#PORTS+=(55912) # LAN2
#PORTS+=(55920)


if [ "x$DOBUILD" == "x1" ]; then
    # build
    dpkg-buildpackage -b --no-sign -nc
fi

if [ "x$?" == "x0" ]; then
    if [ $UPDATE_LINUX -eq 1 ]; then
        for IP in "${IPS[@]}"
        do
            for DESTFILE in "${DESTFILES[@]}"
            do
                echo "===== scp ${DESTFILE} test@${IP}:/home/test/${DESTFILE}"
                scp ${DESTFILE} test@${IP}:/home/test/${DESTFILE}
                echo "===== ssh test@${IP} sudo dpkg -i /home/test/${DESTFILE}"
                ssh test@${IP} sudo dpkg -i /home/test/${DESTFILE}
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
                echo "===== scp -P $PORT ${DESTFILE}  test@${DOREMOTEIP}:/home/test/${DESTFILE}"
                scp -P $PORT ${DESTFILE} test@${DOREMOTEIP}:/home/test/${DESTFILE}
                echo "===== ssh -p $PORT test@${DOREMOTEIP} sudo dpkg -i /home/test/${DESTFILE}"
                ssh -p $PORT test@${DOREMOTEIP} sudo dpkg -i /home/test/${DESTFILE}
            done
        done
    fi
    for IP in "${WIPS[@]}"
    do
        for WINSETUP in "${WDESTFILES[@]}"
        do
            echo "===== scp ${WINSETUP} test@${IP}:D:\\${WINSETUP}"
            scp ${WINSETUP} test@${IP}:D:\\${WINSETUP}
            echo "===== ssh test@${IP} D:\\${WINSETUP} /S"
            ssh test@${IP} D:\\${WINSETUP} /S
        done
    done
    if [ $UPDATE_RVR -eq 1 ]; then
        for PORT in "${RVRPORT[@]}"
        do
            for WINSETUP in "${WDESTFILES[@]}"
            do
                echo "===== scp -P ${PORT} ${WINSETUP} test@${RVRIP}:D:\\${WINSETUP}"
                scp -P ${PORT} ${WINSETUP} test@${RVRIP}:D:\\${WINSETUP}
                echo "===== ssh -P ${PORT} test@${RVRIP} D:\\${WINSETUP} /S"
                ssh -P ${PORT} test@${RVRIP} D:\\${WINSETUP} /S
            done
        done
    fi
fi

# clean up
rm ../*.buildinfo
rm ../*.changes

# windows build
#if [ $DOREMOTE -eq 1 ]; then
#WINSETUP=qiperf-setup-0.2.11306.28.exe
#
#   echo "scp ../${WINSETUP} test@192.168.70.11:D:\\${WINSETUP}"
#    scp ../${WINSETUP} test@192.168.70.11:D:\\${WINSETUP}
#    scp ../${WINSETUP} test@192.168.70.188:D:\\${WINSETUP}
#
    #ssh test@192.168.70.11 D:\\${WINSETUP} /S
#    ssh test@192.168.70.188 D:\\${WINSETUP} /S
#
#fi
