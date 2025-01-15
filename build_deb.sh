#!/bin/bash

BUILDPACKAGES="qiperfd qiperftray"
# Get the machine hardware name
machine_arch=$(uname -m)
if [ "$machine_arch" = "x86_64" ]; then
export QT_SELECT=qt6
BUILDPACKAGES+=" qiperfc"
else
export QT_SELECT=qt5
fi

rm -f *.buildinfo *.changes *.deb

for package in ${BUILDPACKAGES[@]} ; do
    if [ ! -e $package/debian/changelog ]; then
        ln -s debian/changelog $package/debian/
    fi
    cd $package
    #debuild -b -uc -us
    dpkg-buildpackage -b --no-sign -uc
    cd ..
done
