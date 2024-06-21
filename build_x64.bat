@ECHO OFF

set PATH=D:\Qt\5.15.2\mingw81_64\bin;D:\Qt\Tools\mingw810_64\bin;C:\msys64\usr\bin;C:\msys64\mingw64\bin;PATH
set PATH=C:\Program Files (x86)\NSIS\;PATH
make distclean
qmake
make

makensis.exe /V3 qiperf.nsi