@ECHO OFF
setlocal EnableDelayedExpansion

set BUILDAPP=0
if "%1"=="setup" (
    echo only create setup package
) else (
    set BUILDAPP=1
)

if "%BUILDAPP%"=="1" (
    echo "Check Qt build environment"
    echo "%PATH%" | findstr /c:"Qt"
    if !errorlevel! neq 0 (
        set currentDir=%cd%
        set "driveLetter=%currentDir:~0,2%"
        echo set QT msvc2022_64 PATH
        %comspec% /k "C:\Qt\6.8.0\msvc2022_64\bin\qtenv2.bat"
        %driveLetter%
    )
    echo "Check Visual Studio 2022 build environment"
    echo "%PATH%" | findstr /c:"Visual Studio"
    if !errorlevel! neq 0 (
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
