@ECHO OFF
setlocal enabledelayedexpansion

set BUILDAPP=0
if "%1"=="setup" (
    echo only create setup package
) else (
    set BUILDAPP=1
)

if "%BUILDAPP%"=="1" (
    echo "%PATH%" | findstr /c:"Qt" >nul
    if "%errorlevel%" == "1" (
        set currentDir=%cd%
        echo set QT msvc2022_64 PATH
        %comspec% /A /Q /K "C:\Qt\6.8.0\msvc2022_64\bin\qtenv2.bat"
        cd %currentDir%
    )
    echo "%PATH%" | findstr /c:"Visual Studio\2022\Community" >nul
    if "%errorlevel%" == "1" (
        echo set Visual Studio 2022 Community PATH
        %comspec% /k "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
    )

    REM nmake  distclean
    REM del /Q /S qiperfc_x86_64\*
    REM del /Q /S qiperfd_x86_64\*
    REM del /Q /S qiperftray_x86_64\*

    echo qmake...
    qmake
    echo nmake...
    nmake
)

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
    set "APPVERSION=%%i"
    echo !APPVERSION!
)
for /f "tokens=2 delims=\" %%i in ('findstr /c:"%qiperfd_pattern%" "%file%"') do (
    set "QIPERFD_FileVersion=%%i"
    echo !QIPERFD_FileVersion!
)
for /f "tokens=2 delims=\" %%i in ('findstr /c:"%qiperfc_pattern%" "%file%"') do (
    set "QIPERFC_FileVersion=%%i"
    echo !QIPERFC_FileVersion!
)

echo create qiperf daemon setup...
echo "makensis.exe /DAPPVERSION=%APPVERSION% /DAPPFileVersion=%QIPERFD_FileVersion% /DWIN64 /V4 qiperfd.nsi"
makensis.exe /DAPPVERSION=%APPVERSION% /DAPPFileVersion=%QIPERFD_FileVersion% /DWIN64 /V4 qiperfd.nsi
echo create qiperf setup...
echo "makensis.exe /DAPPVERSION=%APPVERSION% /DAPPFileVersion=%QIPERFC_FileVersion% /DWIN64 /V4 qiperf.nsi"
makensis.exe /DAPPVERSION=%APPVERSION% /DAPPFileVersion=%QIPERFC_FileVersion% /DWIN64 /V4 qiperf.nsi
