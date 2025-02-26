@ECHO OFF

REM vc 2022 Community
%comspec% /k "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

REM nmake  distclean
REM del /Q /S qiperfc_x86_64\*
REM del /Q /S qiperfd_x86_64\*
REM del /Q /S qiperftray_x86_64\*

echo qmake...
qmake
echo nmake...
nmake

set PATH=C:\Program Files (x86)\NSIS\;%PATH%
REM # TODO, get version from version.h
set APPVERSION=0.8
set QIPERFD_FileVersion=0.8.11402.25
set QIPERFC_FileVersion=0.8.11402.25

echo create qiperf daemon setup...
makensis.exe /DAPPVERSION=%APPVERSION% /DAPPFileVersion=%QIPERFD_FileVersion% /DWIN64 /V4 qiperfd.nsi
echo create qiperf setup...
makensis.exe /DAPPVERSION=%APPVERSION% /DAPPFileVersion=%QIPERFC_FileVersion% /DWIN64 /V4 qiperf.nsi
