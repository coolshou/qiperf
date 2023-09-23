# qiperf

Quick iperf control tool

# Requirement
 * qtbase5-dev
 * libqt5websockets5-dev
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
	sbuild
# cross-compile for Raspberry 3 (aarch64)
	sudo dpkg --add-architecture arm64
	sudo apt update
	sudo apt-get install build-essential crossbuild-essential-arm64
	cd qiperf
	dpkg-buildpackage -us -uc -b --host-arch arm64
	
sbuild --host=armhf

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
