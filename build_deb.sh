#!/bin/bash


if [ "$1" == "" ]; then
  echo "##### Do not pre clean source tree #####"
  NOTCLEAN="-nc"
else
  echo "##### Do Full compile #####"
  NOTCLEAN=""
fi

BUILDPACKAGES="qiperfd qiperftray"
# Get the machine hardware name
machine_arch=$(uname -m)
if [ "$machine_arch" = "x86_64" ]; then
export QT_SELECT=qt6
BUILDPACKAGES+=" qiperfc"
cp -f qiperfc/debian/control.qt6 qiperfc/debian/control
else
export QT_SELECT=qt5
cp -f qiperfc/debian/control.qt5 qiperfc/debian/control
fi

rm -f *.buildinfo *.changes *.deb
type cmake &> /dev/null;
if [ $? -ne 0 ]; then
    echo "Require cmake to build geographiclib"
    exit
fi
type python3 &> /dev/null;
if [ $? -ne 0 ]; then
    echo "Require python3 to build botan"
    exit
fi
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
  cd lib/qssh
  if [ ! -e botan/configure.py ];then
    git submodule init
    git submodule update
  fi
  cd botan
  python3 ./configure.py --disable-shared-library
  make
  cd ../../../
fi
if [ ! -e lib/qssh/lib/libQSsh.a ]; then
  cd lib/qssh
  #dpkg-buildpackage -b --no-sign
  qmake
  make
  cd ../../
fi
for package in ${BUILDPACKAGES[@]} ; do
    if [ ! -e $package/debian/changelog ]; then
        ln -s debian/changelog $package/debian/
    fi
    cd $package
    #debuild -b -uc -us
    dpkg-buildpackage -b --no-sign ${NOTCLEAN}
    cd ..
done
