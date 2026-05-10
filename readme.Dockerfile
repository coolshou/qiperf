# build ubuntu 24.04 qt6 image
#docker build -t ubuntu-qt6:24.04 -f Dockerfile.ubuntu24.04 .
# only create once
sudo docker buildx build -t ubuntu-qt6:24.04 --file Dockerfile.ubuntu24.04 .

# run when need new deb
docker run --user 1000:1000 \
    --name ubuntu2404-qt6  -it \
    --rm \
    --network host \
    -v /home/coolshou/work/qiperf:/media/qiperf \
    ubuntu-qt6:24.04 /bin/bash

cd /media/qiperf
./build_deb.sh

====================================================================================

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
