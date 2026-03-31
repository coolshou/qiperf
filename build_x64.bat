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
    if errorlevel 1 (
        set currentDir=%cd%
        set driveLetter=%currentDir:~0,2%
        echo set QT msvc2022_64 PATH
        REM this will enter another comspec!
        echo "C:\Qt\6.10.1\msvc2022_64\bin\qtenv2.bat"
        "C:\Qt\6.10.1\msvc2022_64\bin\qtenv2.bat"
        echo "back to %driveLetter%"
        %driveLetter%
    )
    echo "Check Visual Studio 2022 build environment"
    echo "%PATH%" | findstr /c:"Visual Studio"
    if errorlevel 1 (
        echo set Visual Studio 2022 Community PATH "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
        "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
    )

    REM nmake  distclean
    REM del /Q /S qiperfc_x86_64\*
    REM del /Q /S qiperfd_x86_64\*
    REM del /Q /S qiperftray_x86_64\*
    IF NOT EXIST "lib\geographiclib\build\lib\Release\GeographicLib.lib" (
      cd lib\geographiclib
      mkdir build
      cd build
      cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_CXX_FLAGS="/EHsc /wd4819  /wd4456  /wd4244 /WX-"  ..
      msbuild -p:Configuration=Release GeographicLib.sln
      cd ..\..\..\
    )
    IF NOT EXIST "lib\qssh\botan\botan.lib" (
       cd lib\qssh\botan
       python configure.py --cc=msvc --os=windows --cpu=x64 --build-targets="static,shared" --without-documentation --without-sphinx --without-rst2man

       nmake
       # --disable-shared-library
       cd ..\..\..\
    )
    IF NOT EXIST "lib\qssh\lib\libqssh.lib" (
      cd lib\qssh
      qmake
      nmake
      cd ..\..\
    )
    echo qmake...
    qmake
    echo nmake...
    nmake clean
    nmake
    windeployqt6.exe qiperfc_x86_64\qiperfc.exe
    windeployqt6.exe qiperfd_x86_64\qiperfd.exe
    windeployqt6.exe qiperftray_x86_64\qiperftray.exe

)
