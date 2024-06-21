@ECHO OFF

set PATH=D:\Qt\5.15.2\mingw81_32\bin;D:\Qt\Tools\mingw810_32\bin;C:\msys64\usr\bin;C:\msys64\mingw32\bin;%PATH%
make distclean
qmake
make
set PATH=C:\Program Files (x86)\NSIS\;%PATH%
makensis.exe /V3 qiperf.nsi
