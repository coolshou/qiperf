@ECHO OFF

REM vc 2022 Community
%comspec% /k "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

nmake  distclean
del /Q /S qiperfc_x86_64\*
del /Q /S qiperfd_x86_64\*
del /Q /S qiperftray_x86_64\*

qmake
nmake

set PATH=C:\Program Files (x86)\NSIS\;%PATH%
REM # TODO, update version
makensis.exe /DWIN64 /V4 qiperf.nsi
