#/bin/bash

# sudo apt install sshpass

# build deb package
DOBUILD=0
CODENAME=`grep '^VERSION_CODENAME' /etc/os-release | cut -d= -f2`
WINVERSION=`grep '#define QIPERFD_VERSION' src/versions.h | cut -d\"  -f2`
LINUXVERSION=`head -n 1 debian/changelog | awk -F'-' '{print $NF}' | cut -d')' -f1`

VERSION=${WINVERSION}-${LINUXVERSION}
# ================================================

declare -a DESTFILES=()
DESTFILES+=(qiperfd_${VERSION}${CODENAME}_amd64.deb)
DESTFILES+=(qiperftray_${VERSION}${CODENAME}_amd64.deb)

declare -a WDESTFILES=()
WDESTFILES+=(qiperf-setup-${WINVERSION}.exe)

# ================================================
UPDATE_LINUX=1
declare -a IPS=()
INPUT_FILE="deploy.local"
if [[ ! -f "$INPUT_FILE" ]]; then
    #echo "Error: File '$INPUT_FILE' not found."
    #IPS+=("192.168.70.11")
    #IPS+=("192.168.70.12")
    IPS+=("192.168.70.13")
    IPS+=("192.168.70.14")
    #IPS+=("192.168.70.21")
    IPS+=("192.168.70.23")
    IPS+=("192.168.70.24")
    #IPS+=("192.168.70.135")
    #IPS+=("192.168.70.31")
    #IPS+=("192.168.70.32")
    #IPS+=("192.168.70.154")
    #IPS+=("192.168.70.162")
    #IPS+=("192.168.70.147") # not test user
else
    # Read the file line by line
    while IFS= read -r line; do
        # 1. Strip everything from '#' to the end of the line
        clean_line="${line%%#*}"

        # 2. Trim leading/trailing whitespace (optional but highly recommended)
        clean_line=$(echo "$clean_line" | xargs)

        # 3. If the line is not empty, append it to the IPS array
        if [[ -n "$clean_line" ]]; then
            IPS+=("$clean_line")
        fi
    done < "$INPUT_FILE"
fi
echo "Total IPs loaded: ${#IPS[@]}"

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
PORTS+=(55904)
#PORTS+=(55905)
#PORTS+=(55906)
PORTS+=(55908)
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
                echo "===== scp ${DESTFILE} ${USERNAME}@${IP}:/home/test/${DESTFILE}"
                ERROR_OUTPUT=$(scp ${DESTFILE} ${USERNAME}@${IP}:/home/test/${DESTFILE} 2>&1 >/dev/null)
                INSTALL_STATUS=$?
                if [ $INSTALL_STATUS -eq 0 ]; then
                    echo "===== ssh ${USERNAME}@${IP} sshpass -p '123456' sudo dpkg -i /home/test/${DESTFILE}"
                    ERROR_OUTPUT=$(ssh "${USERNAME}@${IP}" "sshpass -p '123456' sudo dpkg -i /home/test/${DESTFILE}" 2>&1 >/dev/null)
                    INSTALL_STATUS=$?
                    if [ $INSTALL_STATUS -ne 0 ]; then
                       echo "***** Fail install /home/test/${DESTFILE} on ${USERNAME}@${IP} *****"
                       echo "Error Details:"
                       echo "$ERROR_OUTPUT"
                    fi
                else
                    echo "**** upload ${DESTFILE} Fail"
                    echo "Error Details:"
                    echo "$ERROR_OUTPUT"
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
                echo "===== scp -P $PORT ${DESTFILE}  ${TARGET}:/home/test/${DESTFILE}"
                ERROR_OUTPUT=$(scp -P $PORT ${DESTFILE} ${TARGET}:/home/test/${DESTFILE} 2>&1 >/dev/null)
                INSTALL_STATUS=$?
                if [ $INSTALL_STATUS -eq 0 ]; then
                    echo "===== ssh -p $PORT ${USERNAME}@${DOREMOTEIP} ${INSTCMD}"
                    ERROR_OUTPUT=$(ssh -p $PORT ${USERNAME}@${DOREMOTEIP} ${INSTCMD} 2>&1 >/dev/null)
                    INSTALL_STATUS=$?
                    if [ $INSTALL_STATUS -ne 0 ]; then
                       echo "***** Fail exec ${INSTCMD} at ${USERNAME}@${DOREMOTEIP}:$PORT *****"
                       echo "Error Details:"
                       echo "$ERROR_OUTPUT"
                    fi
                else
                    echo "upload ${DESTFILE} Fail"
                    echo "Error Details:"
                    echo "$ERROR_OUTPUT"
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
            ERROR_OUTPUT=$(scp ${WINSETUP} ${USERNAME}@${IP}:D:\\${WINSETUP})
            INSTALL_STATUS=$?
            if [ $INSTALL_STATUS -eq 0 ]; then
                echo "===== ssh ${USERNAME}@${IP} D:\\${WINSETUP} /S"
                ERROR_OUTPUT=$(ssh ${USERNAME}@${IP} D:\\${WINSETUP} /S)
                INSTALL_STATUS=$?
                if [ $INSTALL_STATUS -ne 0 ]; then
                    echo "***** Fail exec D:\\${WINSETUP} at ${USERNAME}@${IP} *****"
                    echo "Error Details:"
                    echo "$ERROR_OUTPUT"
                fi
            else
                echo "upload ${WINSETUP} Fail"
                echo "Error Details:"
                echo "$ERROR_OUTPUT"
            fi
        done
    done
    if [ $UPDATE_RVR -eq 1 ]; then
        for PORT in "${RVRPORT[@]}"
        do
            for WINSETUP in "${WDESTFILES[@]}"
            do
                echo "===== scp -P ${PORT} ${WINSETUP} ${USERNAME}@${RVRIP}:D:\\${WINSETUP}"
                ERROR_OUTPUT=$(scp -P ${PORT} ${WINSETUP} ${USERNAME}@${RVRIP}:D:\\${WINSETUP})
                INSTALL_STATUS=$?
                if [ $INSTALL_STATUS -eq 0 ]; then
                    echo "===== ssh -P ${PORT} ${USERNAME}@${RVRIP} D:\\${WINSETUP} /S"
                    ERROR_OUTPUT=$(ssh -P ${PORT} ${USERNAME}@${RVRIP} D:\\${WINSETUP} /S)
                    INSTALL_STATUS=$?
                    if [ $INSTALL_STATUS -ne 0 ]; then
                        echo "***** Fail exec D:\\${WINSETUP} /S at ${USERNAME}@${IP} *****"
                        echo "Error Details:"
                        echo "$ERROR_OUTPUT"
                    fi
                else
                    echo "upload ${WINSETUP} Fail"
                    echo "Error Details:"
                    echo "$ERROR_OUTPUT"
                fi
            done
        done
    fi
fi
