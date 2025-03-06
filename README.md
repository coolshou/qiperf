# qiperf

Quick iperf control tool

# Requirement
 * qt5-qmake | qmake6
 * qtbase5-dev | qt6-base-dev,
 * libqt5websockets5-dev | libqt6websockets6-dev | qt6-websockets-dev
 * libqt5serialport5-dev | libqt6serialport6-dev | qt6-serialport-dev
 * qtwebengine5-dev | qt6-webengine-dev,
 * qtbase5-private-dev | qt6-base-private-dev
 * libsystemd-dev
 * QCustomPlot
 * Qt MaintenanceTool to install android support

# qiperfd
Quick iperf daemon

# qiperftray
Quick iperf tray

[<img src="images/Screenshot_2.jpg" width="500" alt="qiperf on android">]("images/Screenshot_2.jpg")
# qiperfc
Quick iperf console

# Build
	build_deb.sh

# cross-compile for Raspberry 3 (aarch64)
	sudo dpkg --add-architecture arm64
	sudo apt update
	sudo apt-get install build-essential crossbuild-essential-arm64
	cd qiperf
	dpkg-buildpackage -us -uc -b --host-arch arm64
	
sbuild --host=armhf

## Windows	
	requirement
	Qt
	Visual Studio 2022 Community 
	

# Support

* Linux
* Android
* Windows (x64)

# iperf3:

    Android binary:  https://github.com/davidBar-On/android-iperf3

# TODO

* get new apk from internet and update it
* when ip address changed => update UI
* add Mac OS support
* add iOS support
* add Control UI to control multi-iperf at once (like IxChariot)
* add Control UI with throughput chart
