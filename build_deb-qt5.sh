#!/bin/bash

BUILDPACKAGES="qiperfd qiperftray"
BUILDPACKAGES+=" qiperfc"
export QT_SELECT=qt5

rm -f *.buildinfo *.changes *.deb

for package in ${BUILDPACKAGES[@]} ; do
    if [ ! -e $package/debian/changelog ]; then
        ln -s debian/changelog $package/debian/
    fi
    cd $package
    #debuild -b -uc -us
    dpkg-buildpackage -b --no-sign
    cd ..
done
