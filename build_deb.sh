#!/bin/sh

export QT_SELECT=qt6
#export QT_SELECT=qt5

rm *.buildinfo *.changes *.deb

for package in qiperfc qiperfd qiperftray; do
    if [ ! -e $package/debian/changelog ]; then
        ln -s debian/changelog $package/debian/
    fi
    cd $package
    #debuild -b -uc -us
    dpkg-buildpackage -b --no-sign -uc
    cd ..
done
