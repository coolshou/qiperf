#!/bin/bash

BUILDPACKAGES="qiperfd qiperftray"
BUILDPACKAGES+=" qiperfc"
export QT_SELECT=qt5
cp -f qiperfc/debian/control.qt5 qiperfc/debian/control

rm -f *.buildinfo *.changes *.deb
if [ ! -e lib/geographiclib/build/src/libGeographicLib.a ]; then
    cd lib/geographiclib
    if [ ! -e build ]; then
        mkdir build
    fi
    cd build
    cmake -DBUILD_SHARED_LIBS=OFF ..
    make
    cd ../../../
fi
if [ ! -e lib/qssh/botan/libbotan-2.a ]; then
  cd lib/qssh/botan
  python3 ./configure.py --disable-shared-library
  make
  cd ../../../
fi
if [ ! -e lib/qssh/lib/libQSsh.a ]; then
  cd lib/qssh
  dpkg-buildpackage -b --no-sign
  cd ../../
fi
for package in ${BUILDPACKAGES[@]} ; do
    if [ ! -e $package/debian/changelog ]; then
        ln -s debian/changelog $package/debian/
    fi
    cd $package
    #debuild -b -uc -us
    dpkg-buildpackage -b --no-sign
    cd ..
done
