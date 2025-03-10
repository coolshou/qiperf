@ECHO OFF

set BUILDAPP=0
if "%1"=="setup" (
    echo only create setup package
) else (
    set BUILDAPP=1
)

if "%BUILDAPP%"=="1" (
    echo "%PATH%" | findstr /c:"Qt"
    if %errorlevel% EQU 1 (
        set currentDir=%cd%
        echo set QT msvc2022_64 PATH
        %comspec% /A /Q /K "C:\Qt\6.8.0\msvc2022_64\bin\qtenv2.bat"
        cd %currentDir%
    )
    echo "%PATH%" | findstr /c:"Visual Studio"
    if %errorlevel% EQU 1 (
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
