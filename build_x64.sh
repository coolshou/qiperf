#!/bin/sh

export QT_SELECT=qt6
#export QT_SELECT=qt5

for package in qiperfc qiperfd qiperftray; do
	ln -s debian/changelog $package/debian/
    cd $package
    #debuild -b -uc -us
    dpkg-buildpackage -b --no-sign
    cd ..
done
