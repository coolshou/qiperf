@ECHO OFF
setlocal enabledelayedexpansion

echo "%PATH%" | findstr /c:"NSIS" >nul
if "%errorlevel%"=="1" (
    echo setting NSIS PATH
    set "PATH=C:\Program Files (x86)\NSIS\;%PATH%"
)

REM # get version from version.h
set "file=src\versions.h"
set "qiperf_pattern=#define QIPERF_VERSION"
set "qiperfd_pattern=#define QIPERFD_VERSION"
set "qiperfc_pattern=#define QIPERFC_VERSION"

for /f "tokens=2 delims=\" %%i in ('findstr /c:"%qiperf_pattern%" "%file%"') do (
    set APPVERSION=%%i
    echo !APPVERSION!
)
for /f "tokens=2 delims=\" %%i in ('findstr /c:"%qiperfd_pattern%" "%file%"') do (
    set QIPERFD_FileVersion=%%i
    echo !QIPERFD_FileVersion!
)
for /f "tokens=2 delims=\" %%i in ('findstr /c:"%qiperfc_pattern%" "%file%"') do (
    set QIPERFC_FileVersion=%%i
    echo !QIPERFC_FileVersion!
)

IF DEFINED APPVERSION (
    echo create qiperf daemon setup...
    echo "makensis.exe /DAPPVERSION=%APPVERSION% /DAPPFileVersion=%QIPERFD_FileVersion% /DWIN64 /V3 qiperfd.nsi"
    makensis.exe /DAPPVERSION=%APPVERSION% /DAPPFileVersion=%QIPERFD_FileVersion% /DWIN64 /V3 qiperfd.nsi
    echo create qiperf setup...
    echo "makensis.exe /DAPPVERSION=%APPVERSION% /DAPPFileVersion=%QIPERFC_FileVersion% /DWIN64 /V3 qiperf.nsi"
    makensis.exe /DAPPVERSION=%APPVERSION% /DAPPFileVersion=%QIPERFC_FileVersion% /DWIN64 /V3 qiperf.nsi
) ELSE (
    echo "Did not get APPVERSION"
)
