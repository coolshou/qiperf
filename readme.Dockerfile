# build ubuntu 22.04 qt5 image
docker build -t ubuntu-qt5:22.04 -f Dockerfile.ubuntu22.04 .

# run it

docker run --name ubuntu2204-qt5  -it \
    --rm \
    --network host \
    -v /home/jimmy/SOFT/work/qiperf22.04:/media/qiperf \
    ubuntu-qt5:22.04 /bin/bash


cd /media/qiperf
./build_deb-qt5.sh

#copy final deb in /home/jimmy/SOFT/work/qiperf/

##if not using -v, use following to copy file to host
#docker cp <容器ID或名稱>:/path/in/container/your_package.deb /path/on/your/host/


====================================================================================
