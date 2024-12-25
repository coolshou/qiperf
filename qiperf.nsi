; Script generated with the Venis Install Wizard

!addplugindir "nsis\"

; Define your application name
!define APPNAME "qiperf"
!define APPVERSION 0.7
!define APPFileVersion 0.7.11312.25
!define APPDOMAIN "coolshou.idv.tw"
!define APPURL "https://github.com/coolshou/qiperf"
#!define WIN64 ; force  64 bit, comment out for 32 bit
!define QIPERFD_NAME  "qiperfd.exe"
!define QIPERFC_NAME  "qiperfc.exe"
!define QIPERFTRAY_NAME  "qiperftray.exe"
!define SERVICE_WRAPPER "nssm.exe"

!define PRODUCT_REG_KEY "Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
!define PRODUCT_UNINSTALL_EXE "uninstall.exe"

!define VCBUILD 1 ; vs2022
!define QT6
;!define DEBUG
!ifdef DEBUG
 !define DEBUGSTR "d"
!else
 !define DEBUGSTR ""
!endif

VIProductVersion ${APPFileVersion}
var OLD_VERSION
# install mode: 0: daemon only, 1: daemon+ console
var OLD_INSTALL_MODE

!define APPNAMEANDVERSION "qiperf ${APPVERSION}"

; Main Install settings
Name "${APPNAMEANDVERSION}"
InstallDir "$PROGRAMFILES\qiperf"
InstallDirRegKey HKLM "Software\${APPNAME}" ""

!ifdef WIN64
    OutFile "..\qiperf-setup-${APPFileVersion}.exe"
!else
    OutFile "..\qiperf-setup-${APPFileVersion}_x86.exe"
!endif

!include "FileFunc.nsh"
; Use compression
SetCompressor LZMA
!include "x64.nsh"
; Modern interface settings
!include "MUI.nsh"
!include "nsis\nsProcess.nsh"
!include "nsis\FileAssociation.nsh"
!include "WordFunc.nsh"
!insertmacro VersionCompare

!define MUI_ABORTWARNING
!define MUI_ICON "images\qiperf.ico"
# uninstall icon
!define MUI_UNICON "images\uninstall.ico"
!insertmacro MUI_PAGE_WELCOME
Page custom uninstallold ;Custom page
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Set languages (first is default language)
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_RESERVEFILE_LANGDLL

VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${APPNAME}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${APPVERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "Comments" "Quick iperf console and daemon"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "${APPDOMAIN}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalTrademarks" "${APPNAME} is a trademark of ${APPDOMAIN}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "(C) ${APPDOMAIN}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "${APPNAME}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${APPFileVersion}"

Function uninstallold
    # uninstall old version
    ${If} $OLD_VERSION != ""
        ExecWait "$OLD_VERSION"
    ${EndIf}

FunctionEnd

Section "qiperf daemon" SECTION_Daemon
    ; Set Section properties
    SetOverwrite on

    ; Set Section Files and Shortcuts
    SetOutPath "$INSTDIR\"
    File "images\qiperf.ico"
!ifdef WIN64
    !cd "qiperfd_x86_64"
!else
    !cd "qiperfd_x86"
!endif
!ifdef VCBUILD
!ifdef WIN64
    File "vc_redist.x64.exe"
!else
    File "vc_redist.x86.exe"
!endif
    ;File "concrt140${DEBUGSTR}.dll"
    ;File "msvcp140_1${DEBUGSTR}.dll"
    ;File "msvcp140_2${DEBUGSTR}.dll"
    ;File "msvcp140${DEBUGSTR}.dll"
    ;File "msvcp140${DEBUGSTR}_atomic_wait.dll"
    ;File "msvcp140${DEBUGSTR}_codecvt_ids.dll"
    ;File "vccorlib140${DEBUGSTR}.dll"
    ;File "vcruntime140_1${DEBUGSTR}.dll"
    ;File "vcruntime140_threads${DEBUGSTR}.dll"
    ;File "vcruntime140${DEBUGSTR}.dll"

!else
    ;;mingw
    File "libgcc_s_seh-1.dll"
    File "libstdc++-6.dll"
    File "libwinpthread-1.dll"
    File "libEGL.dll"
    File "libGLESv2.dll"

!endif
    File "${QIPERFD_NAME}"
!ifdef QT6
    File "Qt6Core${DEBUGSTR}.dll"
    File "Qt6Network${DEBUGSTR}.dll"
    File "Qt6SerialPort${DEBUGSTR}.dll"
    File "Qt6WebSockets${DEBUGSTR}.dll"
    SetOutPath "$INSTDIR\networkinformation\"
    File "networkinformation\qnetworklistmanager${DEBUGSTR}.dll"
    SetOutPath "$INSTDIR\tls"
    File "tls\qcertonlybackend${DEBUGSTR}.dll"
    File "tls\qschannelbackend${DEBUGSTR}.dll"
!else
    ;qt5
    File "Qt5Core.dll"
    File "Qt5Network.dll"
    File "Qt5WebSockets.dll"
    SetOutPath "$INSTDIR\bearer\"
    File "bearer\qgenericbearer.dll"
    File "Qt5Gui.dll"
    File "Qt5Svg.dll"
    File "Qt5Widgets.dll"

!endif
    SetOutPath "$INSTDIR\translations\"
    File "translations\qt_ar.qm"
    File "translations\qt_bg.qm"
    File "translations\qt_ca.qm"
    File "translations\qt_cs.qm"
    File "translations\qt_da.qm"
    File "translations\qt_de.qm"
    File "translations\qt_en.qm"
    File "translations\qt_es.qm"
    File "translations\qt_fi.qm"
    File "translations\qt_fr.qm"
    File "translations\qt_gd.qm"
    File "translations\qt_he.qm"
    File "translations\qt_hu.qm"
    File "translations\qt_it.qm"
    File "translations\qt_ja.qm"
    File "translations\qt_ko.qm"
    File "translations\qt_lv.qm"
    File "translations\qt_pl.qm"
    File "translations\qt_ru.qm"
    File "translations\qt_sk.qm"
    File "translations\qt_tr.qm"
    File "translations\qt_uk.qm"
    File "translations\qt_zh_TW.qm"
    SetOutPath "$INSTDIR\windows\x86\"
    File "windows\x86\cygcrypto-1.1.dll"
    File "windows\x86\cyggcc_s-1.dll"
    File "windows\x86\cygwin1.dll"
    File "windows\x86\cygz.dll"
    File "windows\x86\iperf2.exe"
    File "windows\x86\iperf2.1.exe"
    File "windows\x86\iperf3.exe"
    SetOutPath "$INSTDIR\windows\x86_64\"
    File "windows\x86_64\cygcrypto-1.1.dll"
    File "windows\x86_64\cygwin1.dll"
    File "windows\x86_64\cygz.dll"
    File "windows\x86_64\iperf3.exe"
    SetOutPath "$INSTDIR\"
    !cd ..
!ifdef WIN64
    !cd qiperftray_x86_64
!else
    !cd qiperftray_x86
!endif

    File "D3Dcompiler_47.dll"
    File "opengl32sw.dll"
!ifdef VCBUILD
    File "dxil.dll"
!endif
    File "${QIPERFTRAY_NAME}"
!ifdef QT6
    File "Qt6Gui${DEBUGSTR}.dll"
    File "Qt6Widgets${DEBUGSTR}.dll"
    SetOutPath "$INSTDIR\generic\"
    File "generic\qtuiotouchplugin${DEBUGSTR}.dll"
!endif
    SetOutPath "$INSTDIR\iconengines\"
    File "iconengines\qsvgicon${DEBUGSTR}.dll"
    SetOutPath "$INSTDIR\imageformats\"
    File "imageformats\qgif${DEBUGSTR}.dll"
    File "imageformats\qicns${DEBUGSTR}.dll"
    File "imageformats\qico${DEBUGSTR}.dll"
    File "imageformats\qjpeg${DEBUGSTR}.dll"
    File "imageformats\qsvg${DEBUGSTR}.dll"
    File "imageformats\qtga${DEBUGSTR}.dll"
    File "imageformats\qtiff${DEBUGSTR}.dll"
    File "imageformats\qwbmp${DEBUGSTR}.dll"
    File "imageformats\qwebp${DEBUGSTR}.dll"
    SetOutPath "$INSTDIR\platforms\"
    File "platforms\qwindows${DEBUGSTR}.dll"
    SetOutPath "$INSTDIR\styles\"
!ifdef QT6
    File "styles\qmodernwindowsstyle${DEBUGSTR}.dll"
!else
    File "styles\qwindowsvistastyle.dll"
!endif
    !cd ..
    # #  serivice file
    SetOutPath "$INSTDIR"
!ifdef WIN64
    File "lib\nssm.exe"
!else
    File "lib\nssm_x86.exe" /oname=nssm.exe
!endif
    #CreateShortCut "$DESKTOP\qiperftray.lnk" "$INSTDIR\${QIPERFTRAY_NAME}"

    CreateDirectory "$SMPROGRAMS\qiperf"
    #CreateShortCut "$SMPROGRAMS\qiperf\qiperfd.lnk" "$INSTDIR\${QIPERFD_NAME}"
    CreateShortCut "$SMPROGRAMS\qiperf\qiperftray.lnk" "$INSTDIR\${QIPERFTRAY_NAME}"
    CreateShortCut "$SMPROGRAMS\qiperf\Uninstall.lnk" "$INSTDIR\${PRODUCT_UNINSTALL_EXE}"

    WriteRegStr HKLM "Software\${PRODUCT_REG_KEY}" "InstallMode" "0"
    # set QIPERFTRAY_NAME run on system boot
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "${QIPERFTRAY_NAME}" '"$INSTDIR\${QIPERFTRAY_NAME}"'

    Call check_vc_redist
    Call install_qiperfd
SectionEnd

Section "qiperf console" SECTION_Console
    #TODO: close qiperfc before copy new file

    ; Set Section properties
    SetOverwrite on

    ; Set Section Files and Shortcuts
    SetOutPath "$INSTDIR\"
!ifdef WIN64
    !cd qiperfc_x86_64
!else
    !cd qiperfc_x86
!endif
    File "${QIPERFC_NAME}"
!ifdef QT6
    File "Qt6Core5Compat${DEBUGSTR}.dll"
    File "Qt6OpenGL${DEBUGSTR}.dll"
    File "Qt6Positioning${DEBUGSTR}.dll"
    File "Qt6PrintSupport${DEBUGSTR}.dll"
    File "Qt6Qml${DEBUGSTR}.dll"
    File "Qt6QmlMeta${DEBUGSTR}.dll"
    File "Qt6QmlModels${DEBUGSTR}.dll"
    File "Qt6QmlWorkerScript${DEBUGSTR}.dll"
    File "Qt6Quick3DUtils${DEBUGSTR}.dll"
    File "Qt6Quick${DEBUGSTR}.dll"
    File "Qt6QuickWidgets${DEBUGSTR}.dll"
    File "Qt6Svg${DEBUGSTR}.dll"
    File "Qt6VirtualKeyboard${DEBUGSTR}.dll"
    File "Qt6WebChannel${DEBUGSTR}.dll"
    File "Qt6WebEngineCore${DEBUGSTR}.dll"
    File "Qt6WebEngineWidgets${DEBUGSTR}.dll"
    File "QtWebEngineProcess${DEBUGSTR}.exe"
    SetOutPath "$INSTDIR\platforminputcontexts"
    File "platforminputcontexts\qtvirtualkeyboardplugin${DEBUGSTR}.dll"
    SetOutPath "$INSTDIR\position"
    File "position\qtposition_nmea${DEBUGSTR}.dll"
    File "position\qtposition_positionpoll${DEBUGSTR}.dll"
    File "position\qtposition_winrt${DEBUGSTR}.dll"
    SetOutPath "$INSTDIR\qml"
    SetOutPath "$INSTDIR\qmltooling"
    File "qmltooling\qmldbg_debugger${DEBUGSTR}.dll"
    File "qmltooling\qmldbg_inspector${DEBUGSTR}.dll"
    File "qmltooling\qmldbg_local${DEBUGSTR}.dll"
    File "qmltooling\qmldbg_messages${DEBUGSTR}.dll"
    File "qmltooling\qmldbg_native${DEBUGSTR}.dll"
    File "qmltooling\qmldbg_nativedebugger${DEBUGSTR}.dll"
    File "qmltooling\qmldbg_preview${DEBUGSTR}.dll"
    File "qmltooling\qmldbg_profiler${DEBUGSTR}.dll"
    File "qmltooling\qmldbg_quick3dprofiler${DEBUGSTR}.dll"
    File "qmltooling\qmldbg_quickprofiler${DEBUGSTR}.dll"
    File "qmltooling\qmldbg_server${DEBUGSTR}.dll"
    File "qmltooling\qmldbg_tcp${DEBUGSTR}.dll"
    SetOutPath "$INSTDIR\resources"
    File "resources\icudtl.dat"
    File "resources\qtwebengine_devtools_resources.pak"
    File "resources\qtwebengine_resources.pak"
    File "resources\qtwebengine_resources_100p.pak"
    File "resources\qtwebengine_resources_200p.pak"
!ifdef DEBUG
    File "resources\v8_context_snapshot.debug.bin"
!else
    File "resources\v8_context_snapshot.bin"
!endif
    SetOutPath "$INSTDIR\translations\qtwebengine_locales"
    File "translations\qtwebengine_locales\am.pak"
    File "translations\qtwebengine_locales\ar.pak"
    File "translations\qtwebengine_locales\bg.pak"
    File "translations\qtwebengine_locales\bn.pak"
    File "translations\qtwebengine_locales\ca.pak"
    File "translations\qtwebengine_locales\cs.pak"
    File "translations\qtwebengine_locales\da.pak"
    File "translations\qtwebengine_locales\de.pak"
    File "translations\qtwebengine_locales\el.pak"
    File "translations\qtwebengine_locales\en-GB.pak"
    File "translations\qtwebengine_locales\en-US.pak"
    File "translations\qtwebengine_locales\es-419.pak"
    File "translations\qtwebengine_locales\es.pak"
    File "translations\qtwebengine_locales\et.pak"
    File "translations\qtwebengine_locales\fa.pak"
    File "translations\qtwebengine_locales\fi.pak"
    File "translations\qtwebengine_locales\fil.pak"
    File "translations\qtwebengine_locales\fr.pak"
    File "translations\qtwebengine_locales\gu.pak"
    File "translations\qtwebengine_locales\he.pak"
    File "translations\qtwebengine_locales\hi.pak"
    File "translations\qtwebengine_locales\hr.pak"
    File "translations\qtwebengine_locales\hu.pak"
    File "translations\qtwebengine_locales\id.pak"
    File "translations\qtwebengine_locales\it.pak"
    File "translations\qtwebengine_locales\ja.pak"
    File "translations\qtwebengine_locales\kn.pak"
    File "translations\qtwebengine_locales\ko.pak"
    File "translations\qtwebengine_locales\lt.pak"
    File "translations\qtwebengine_locales\lv.pak"
    File "translations\qtwebengine_locales\ml.pak"
    File "translations\qtwebengine_locales\mr.pak"
    File "translations\qtwebengine_locales\ms.pak"
    File "translations\qtwebengine_locales\nb.pak"
    File "translations\qtwebengine_locales\nl.pak"
    File "translations\qtwebengine_locales\pl.pak"
    File "translations\qtwebengine_locales\pt-BR.pak"
    File "translations\qtwebengine_locales\pt-PT.pak"
    File "translations\qtwebengine_locales\ro.pak"
    File "translations\qtwebengine_locales\ru.pak"
    File "translations\qtwebengine_locales\sk.pak"
    File "translations\qtwebengine_locales\sl.pak"
    File "translations\qtwebengine_locales\sr.pak"
    File "translations\qtwebengine_locales\sv.pak"
    File "translations\qtwebengine_locales\sw.pak"
    File "translations\qtwebengine_locales\ta.pak"
    File "translations\qtwebengine_locales\te.pak"
    File "translations\qtwebengine_locales\th.pak"
    File "translations\qtwebengine_locales\tr.pak"
    File "translations\qtwebengine_locales\uk.pak"
    File "translations\qtwebengine_locales\vi.pak"
    File "translations\qtwebengine_locales\zh-CN.pak"
    File "translations\qtwebengine_locales\zh-TW.pak"
!else
    File "Qt5PrintSupport.dll"
!endif
    SetOutPath "$INSTDIR\template\"
    File "..\qiperfc\template\result.html"
!ifdef QT6
!else
    SetOutPath "$INSTDIR\printsupport\"
    File "printsupport\windowsprintersupport.dll"
!endif
    !cd ..
    CreateShortCut "$DESKTOP\qiperfc.lnk" "$INSTDIR\${QIPERFC_NAME}"
    CreateShortCut "$SMPROGRAMS\qiperf\qiperfc.lnk" "$INSTDIR\${QIPERFC_NAME}"

    WriteRegStr HKLM "Software\${PRODUCT_REG_KEY}" "InstallMode" "1"
    # reg ".qip" ext
    ${registerExtension} "$INSTDIR\${QIPERFC_NAME}" ".qip" "Quick Iperf config File"
    SimpleFC::AddApplication "qiperf console" "$INSTDIR\${QIPERFC_NAME}" 0 2 "" 1
    Pop $0 ; return error(1)/success(0)

SectionEnd

Section -FinishSection
    WriteRegStr HKLM "Software\${APPNAME}" "" "$INSTDIR"
    WriteRegStr HKLM "Software\${PRODUCT_REG_KEY}" "DisplayName" "${APPNAME}"
    WriteRegStr HKLM "Software\${PRODUCT_REG_KEY}" "DisplayIcon" "$INSTDIR\qiperf.ico"
    WriteRegStr HKLM "Software\${PRODUCT_REG_KEY}" "Publisher" "${APPDOMAIN}"
    WriteRegStr HKLM "Software\${PRODUCT_REG_KEY}" "DisplayVersion" "${APPFileVersion}"
    WriteRegStr HKLM "Software\${PRODUCT_REG_KEY}" "HelpLink" "${APPURL}"
    WriteRegDWORD HKLM "Software\${PRODUCT_REG_KEY}" "NoModify" "1"
    WriteRegDWORD HKLM "Software\${PRODUCT_REG_KEY}" "NoRepair" "1"
    WriteRegStr HKLM "Software\${PRODUCT_REG_KEY}" "UninstallString" "$INSTDIR\${PRODUCT_UNINSTALL_EXE}"
    WriteRegStr HKLM "Software\${PRODUCT_REG_KEY}" "QuietUninstallString" '"$INSTDIR\${PRODUCT_UNINSTALL_EXE}" /S _?=$INSTDIR'

    WriteUninstaller "$INSTDIR\${PRODUCT_UNINSTALL_EXE}"
    # size
    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    IntFmt $0 "0x%08X" $0
    WriteRegDWORD HKLM "Software\${PRODUCT_REG_KEY}" "EstimatedSize" "$0"
SectionEnd

; Modern install component descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SECTION_Daemon} "quick iperf daemon && systray"
    !insertmacro MUI_DESCRIPTION_TEXT ${SECTION_Console} "quick iperf console"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;Uninstall section
Section Uninstall
    Call un.install_qiperfd
    ReadRegStr $R0 HKLM "Software\${PRODUCT_REG_KEY}" "InstallMode"
    strcpy $OLD_INSTALL_MODE $R0
    ${If} $OLD_INSTALL_MODE  == "1"
    Call un.install_qiperfc
    ${EndIf}
!ifdef WIN64
        SetRegView 64
!endif
    ;Remove from registry...
    DeleteRegKey HKLM "Software\${PRODUCT_REG_KEY}"
    DeleteRegKey HKLM "SOFTWARE\${APPNAME}"

    ; Delete self
    Delete "$INSTDIR\${PRODUCT_UNINSTALL_EXE}"
    Delete "$INSTDIR\qiperf.ico"
    ; Delete Shortcuts
    #Delete "$DESKTOP\qiperftray.lnk"
    Delete "$DESKTOP\qiperfc.lnk"
    Delete "$SMPROGRAMS\qiperf\qiperfd.lnk"
    Delete "$SMPROGRAMS\qiperf\qiperftray.lnk"
    Delete "$SMPROGRAMS\qiperf\qiperfc.lnk"
    Delete "$SMPROGRAMS\qiperf\Uninstall.lnk"

    ; Clean up qiperf daemon
!ifdef VCBUILD
!ifdef WIN64
    Delete "$INSTDIR\vc_redist.x64.exe"
!else
    Delete "$INSTDIR\vc_redist.x86.exe"
!endif
    ;Delete "$INSTDIR\concrt140${DEBUGSTR}.dll"
    ;Delete "$INSTDIR\msvcp140_1${DEBUGSTR}.dll"
    ;Delete "$INSTDIR\msvcp140_2${DEBUGSTR}.dll"
    ;Delete "$INSTDIR\msvcp140${DEBUGSTR}.dll"
    ;Delete "$INSTDIR\msvcp140${DEBUGSTR}_atomic_wait.dll"
    ;Delete "$INSTDIR\msvcp140${DEBUGSTR}_codecvt_ids.dll"
    ;Delete "$INSTDIR\vccorlib140${DEBUGSTR}.dll"
    ;Delete "$INSTDIR\vcruntime140_1${DEBUGSTR}.dll"
    ;Delete "$INSTDIR\vcruntime140_threads${DEBUGSTR}.dll"
    ;Delete "$INSTDIR\vcruntime140${DEBUGSTR}.dll"
    Delete "$INSTDIR\dxil.dll"
!else
    Delete "$INSTDIR\libgcc_s_seh-1.dll"
    Delete "$INSTDIR\libstdc++-6.dll"
    Delete "$INSTDIR\libwinpthread-1.dll"
    Delete "$INSTDIR\libEGL.dll"
    Delete "$INSTDIR\libGLESv2.dll"

!endif
    Delete "$INSTDIR\${QIPERFD_NAME}"
!ifdef QT6
    Delete "$INSTDIR\Qt6Core${DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6Network${DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6SerialPort${DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6WebSockets${DEBUGSTR}.dll"
    Delete "$INSTDIR\networkinformation\qnetworklistmanager${DEBUGSTR}.dll"
    Delete "$INSTDIR\tls\qcertonlybackend${DEBUGSTR}.dll"
    Delete "$INSTDIR\tls\qschannelbackend${DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6Gui${DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6Widgets${DEBUGSTR}.dll"
    Delete "$INSTDIR\generic\qtuiotouchplugin${DEBUGSTR}.dll"
    Delete "$INSTDIR\styles\qmodernwindowsstyle{DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6Core5Compat{DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6OpenGL{DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6Positioning{DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6PrintSupport{DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6Qml{DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6QmlMeta{DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6QmlModels{DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6QmlWorkerScript{DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6Quick3DUtils{DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6Quick{DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6QuickWidgets{DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6SerialPort{DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6Svg{DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6VirtualKeyboard{DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6WebChannel{DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6WebEngineCore{DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6WebEngineWidgets{DEBUGSTR}.dll"
    Delete "$INSTDIR\QtWebEngineProcessd.exe"
    Delete "$INSTDIR\platforminputcontexts\qtvirtualkeyboardplugin{DEBUGSTR}.dll"
    Delete "$INSTDIR\position\qtposition_nmea{DEBUGSTR}.dll"
    Delete "$INSTDIR\position\qtposition_positionpoll{DEBUGSTR}.dll"
    Delete "$INSTDIR\position\qtposition_winrt{DEBUGSTR}.dll"
    Delete "$INSTDIR\qmltooling\qmldbg_debugger{DEBUGSTR}.dll"
    Delete "$INSTDIR\qmltooling\qmldbg_inspector{DEBUGSTR}.dll"
    Delete "$INSTDIR\qmltooling\qmldbg_local{DEBUGSTR}.dll"
    Delete "$INSTDIR\qmltooling\qmldbg_messages{DEBUGSTR}.dll"
    Delete "$INSTDIR\qmltooling\qmldbg_native{DEBUGSTR}.dll"
    Delete "$INSTDIR\qmltooling\qmldbg_nativedebugger{DEBUGSTR}.dll"
    Delete "$INSTDIR\qmltooling\qmldbg_preview{DEBUGSTR}.dll"
    Delete "$INSTDIR\qmltooling\qmldbg_profiler{DEBUGSTR}.dll"
    Delete "$INSTDIR\qmltooling\qmldbg_quick3dprofiler{DEBUGSTR}.dll"
    Delete "$INSTDIR\qmltooling\qmldbg_quickprofiler{DEBUGSTR}.dll"
    Delete "$INSTDIR\qmltooling\qmldbg_server{DEBUGSTR}.dll"
    Delete "$INSTDIR\qmltooling\qmldbg_tcp{DEBUGSTR}.dll"
    Delete "$INSTDIR\resources\icudtl.dat"
    Delete "$INSTDIR\resources\qtwebengine_devtools_resources.pak"
    Delete "$INSTDIR\resources\qtwebengine_resources.pak"
    Delete "$INSTDIR\resources\qtwebengine_resources_100p.pak"
    Delete "$INSTDIR\resources\qtwebengine_resources_200p.pak"
!ifdef DEBUG
    Delete "$INSTDIR\resources\v8_context_snapshot.debug.bin"
!else
    Delete "$INSTDIR\resources\v8_context_snapshot.bin"
!endif
    Delete "$INSTDIR\styles\qmodernwindowsstyle{DEBUGSTR}.dll"
    Delete "$INSTDIR\translations\qtwebengine_locales\am.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\ar.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\bg.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\bn.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\ca.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\cs.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\da.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\de.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\el.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\en-GB.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\en-US.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\es-419.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\es.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\et.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\fa.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\fi.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\fil.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\fr.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\gu.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\he.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\hi.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\hr.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\hu.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\id.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\it.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\ja.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\kn.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\ko.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\lt.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\lv.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\ml.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\mr.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\ms.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\nb.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\nl.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\pl.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\pt-BR.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\pt-PT.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\ro.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\ru.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\sk.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\sl.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\sr.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\sv.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\sw.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\ta.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\te.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\th.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\tr.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\uk.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\vi.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\zh-CN.pak"
    Delete "$INSTDIR\translations\qtwebengine_locales\zh-TW.pak"
!else
    Delete "$INSTDIR\Qt5Core.dll"
    Delete "$INSTDIR\Qt5Network.dll"
    Delete "$INSTDIR\Qt5WebSockets.dll"
    Delete "$INSTDIR\Qt5Gui.dll"
    Delete "$INSTDIR\Qt5Svg.dll"
    Delete "$INSTDIR\Qt5Widgets.dll"
    Delete "$INSTDIR\bearer\qgenericbearer.dll"
    Delete "$INSTDIR\styles\qwindowsvistastyle.dll"
    Delete "$INSTDIR\Qt5PrintSupport.dll"
    Delete "$INSTDIR\printsupport\windowsprintersupport.dll"
!endif

    Delete "$INSTDIR\translations\qt_ar.qm"
    Delete "$INSTDIR\translations\qt_bg.qm"
    Delete "$INSTDIR\translations\qt_ca.qm"
    Delete "$INSTDIR\translations\qt_cs.qm"
    Delete "$INSTDIR\translations\qt_da.qm"
    Delete "$INSTDIR\translations\qt_de.qm"
    Delete "$INSTDIR\translations\qt_en.qm"
    Delete "$INSTDIR\translations\qt_es.qm"
    Delete "$INSTDIR\translations\qt_fi.qm"
    Delete "$INSTDIR\translations\qt_fr.qm"
    Delete "$INSTDIR\translations\qt_gd.qm"
    Delete "$INSTDIR\translations\qt_he.qm"
    Delete "$INSTDIR\translations\qt_hu.qm"
    Delete "$INSTDIR\translations\qt_it.qm"
    Delete "$INSTDIR\translations\qt_ja.qm"
    Delete "$INSTDIR\translations\qt_ko.qm"
    Delete "$INSTDIR\translations\qt_lv.qm"
    Delete "$INSTDIR\translations\qt_pl.qm"
    Delete "$INSTDIR\translations\qt_ru.qm"
    Delete "$INSTDIR\translations\qt_sk.qm"
    Delete "$INSTDIR\translations\qt_tr.qm"
    Delete "$INSTDIR\translations\qt_uk.qm"
    Delete "$INSTDIR\translations\qt_zh_TW.qm"
    Delete "$INSTDIR\windows\x86\cygcrypto-1.1.dll"
    Delete "$INSTDIR\windows\x86\cyggcc_s-1.dll"
    Delete "$INSTDIR\windows\x86\cygwin1.dll"
    Delete "$INSTDIR\windows\x86\cygz.dll"
    Delete "$INSTDIR\windows\x86\iperf2.exe"
    Delete "$INSTDIR\windows\x86\iperf2.1.exe"
    Delete "$INSTDIR\windows\x86\iperf3.exe"
    Delete "$INSTDIR\windows\x86_64\cygcrypto-1.1.dll"
    Delete "$INSTDIR\windows\x86_64\cygwin1.dll"
    Delete "$INSTDIR\windows\x86_64\cygz.dll"
    Delete "$INSTDIR\windows\x86_64\iperf3.exe"

    Delete "$INSTDIR\D3Dcompiler_47.dll"
    Delete "$INSTDIR\opengl32sw.dll"
    Delete "$INSTDIR\${QIPERFTRAY_NAME}"
    Delete "$INSTDIR\nssm.exe"
    Delete "$INSTDIR\iconengines\qsvgicon.dll"
    Delete "$INSTDIR\imageformats\qgif.dll"
    Delete "$INSTDIR\imageformats\qicns.dll"
    Delete "$INSTDIR\imageformats\qico.dll"
    Delete "$INSTDIR\imageformats\qjpeg.dll"
    Delete "$INSTDIR\imageformats\qsvg.dll"
    Delete "$INSTDIR\imageformats\qtga.dll"
    Delete "$INSTDIR\imageformats\qtiff.dll"
    Delete "$INSTDIR\imageformats\qwbmp.dll"
    Delete "$INSTDIR\imageformats\qwebp.dll"
    Delete "$INSTDIR\platforms\qwindows.dll"
    ; Clean up qiperf console
    Delete "$INSTDIR\${QIPERFC_NAME}"
    Delete "$INSTDIR\template\result.html"

    ; Remove remaining directories
    RMDir "$SMPROGRAMS\qiperf"
    RMDir "$INSTDIR\windows\x86_64\"
    RMDir "$INSTDIR\windows\x86\"
    RMDir "$INSTDIR\windows\"
!ifdef QT6
    RMDir "$INSTDIR\networkinformation\"
    RMDir "$INSTDIR\tls\"
    RMDir "$INSTDIR\generic\"
    RMDir "$INSTDIR\platforminputcontexts\"
    RMDir "$INSTDIR\position\"
    RMDir "$INSTDIR\qml\"
    RMDir "$INSTDIR\qmltooling\"
    RMDir "$INSTDIR\resources\"
    RMDir "$INSTDIR\translations\qtwebengine_locales\"
!else
    RMDir "$INSTDIR\bearer\"
!endif
    RMDir "$INSTDIR\translations\"
    RMDir "$INSTDIR\styles\"
    RMDir "$INSTDIR\printsupport\"
    RMDir "$INSTDIR\platforms\"
    RMDir "$INSTDIR\imageformats\"
    RMDir "$INSTDIR\iconengines\"
    RMDir "$INSTDIR\template"
    RMDir "$INSTDIR\"

    ${unregisterExtension} ".qip" "Quick Iperf config File"

SectionEnd

BrandingText "Quick iperf daemon"

Function .onInit
# TODO: Silent mode/ Full mode
    ${If} ${RunningX64}
    !ifdef WIN64
            SetRegView 64
    !endif
    ${Else}
    !ifdef WIN64
            MessageBox MB_OK|MB_ICONSTOP 'This is the 64 bit ${APPNAME} installer$\r$\nPlease download the 32 bit version $\r$\nClick Ok to quit Setup.'
            Quit
    !endif
    ${EndIf}

# ;Check earlier installation
    ClearErrors
    ReadRegStr $0 HKLM "Software\${PRODUCT_REG_KEY}" "DisplayVersion"
    IfErrors init.uninst ; older versions might not have "Version" string set
    ${VersionCompare} $0 ${APPFileVersion} $1
    IntCmp $1 2 init.uninst
      MessageBox MB_YESNO|MB_ICONQUESTION "${APPNAME} version $0 seems to be already installed on your system.$\nWould you like to proceed with the installation of version ${APPFileVersion}?" \
        IDYES init.uninst
    Quit

init.uninst:
    ClearErrors
#${If} ${Silent}
    ReadRegStr $R0 HKLM "Software\${PRODUCT_REG_KEY}" "QuietUninstallString"
#${Else}
#    ReadRegStr $R0 HKLM "Software\${PRODUCT_REG_KEY}" "UninstallString"
#${EndIf}
    IfErrors init.done
    strcpy $OLD_VERSION $R0
    #ExecWait "$R0"

init.done:
    # TODO: get old setup mode

    # get previous install mode
    ClearErrors
    ReadRegStr $R0 HKLM "Software\${PRODUCT_REG_KEY}" "InstallMode"
    strcpy $OLD_INSTALL_MODE $R0

    !ifdef WIN64
      strcpy $INSTDIR "$PROGRAMFILES64\${APPNAME}"
    !endif

    # set section 'daemon' as selected and read-only
    IntOp $0 ${SF_SELECTED} | ${SF_RO}
    SectionSetFlags ${SECTION_Daemon} $0
   # set section 'console' as unselected
   #IntOp $0 ~${SF_SELECTED}
   ${If} $OLD_INSTALL_MODE  == "1"
       SectionSetFlags ${SECTION_Console}  ${SF_SELECTED}
   ${Else}
      SectionSetFlags ${SECTION_Console} 0
   ${EndIf}
FunctionEnd

Function install_qiperfd
    ; Add an application to the firewall exception list - All Networks - All IP Version - Enabled
    SimpleFC::AddApplication "qiperfd daemon" "$INSTDIR\${QIPERFD_NAME}" 0 2 "" 1
    Pop $0 ; return error(1)/success(0)
    ; Add iperf3 to firewall
    !ifdef WIN64
    SimpleFC::AddApplication "iperf3" "$INSTDIR\x86_64\iperf3.exe" 0 2 "" 1
    !else
    SimpleFC::AddApplication "iperf3" "$INSTDIR\x86\iperf3.exe" 0 2 "" 1
    !endif
    Pop $0 ; return error(1)/success(0)
    ; Add iperf2.1 to firewall
    SimpleFC::AddApplication "iperf2.1" "$INSTDIR\x86\iperf2.1.exe" 0 2 "" 1
    Pop $0 ; return error(1)/success(0)
    ; Add iperf2 to firewall
    SimpleFC::AddApplication "iperf2" "$INSTDIR\x86\iperf2.exe" 0 2 "" 1
    Pop $0 ; return error(1)/success(0)

    # install qiperfd  service & start it
    Exec '"$INSTDIR\nssm.exe" install "qiperfd" "$INSTDIR\${QIPERFD_NAME}"'
    Exec '"$INSTDIR\nssm.exe" start "qiperfd"'
FunctionEnd

Function install_vc_redist
    ;not installed, so run the installer in quiet mode
    !ifdef WIN64
    ExecWait '$INSTDIR\vc_redist.x64.exe /q /norestart'
    !else
    ExecWait '$INSTDIR\vc_redist.x86.exe /q /norestart'
    !endif
FunctionEnd

Function check_vc_redist
    ${If} ${RunningX64}
            ReadRegStr $1 HKLM "SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64" "Installed"
            StrCmp $1 1 installed install_vc_redist
    ${Else}
            ReadRegStr $1 HKLM "SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x86" "Installed"
            StrCmp $1 1 installed install_vc_redist
    ${EndIf}

    installed:
    ; Visual Studio 2017 version 15.6 introduced msvcp140_1
    ; Visual Studio 2019 ? introduced msvcp140_2
    ; 14.24 OKs

    ;check vc_redist version <14.20 is not good
    ${If} ${RunningX64}
            ReadRegStr $1 HKLM "SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x64" "Minor"
            ;StrCmp $1 24 version_ok
    ${Else}
            ReadRegStr $1 HKLM "SOFTWARE\Microsoft\VisualStudio\14.0\VC\Runtimes\x86" "Minor"
            ;StrCmp $1 24 version_ok
    ${EndIf}
    IntCmp $1 24 issame install_vc_redist morethan
    install_vc_redist:
    ;${If} $var >= 2
        call install_vc_redist
    ;${EndIf}

    issame:
    morethan:
    ;we are done

FunctionEnd

Function un.install_qiperfd
    # uninstall qiperfd  service
    Exec '"$INSTDIR\nssm.exe" stop "qiperfd"'
    Exec '"$INSTDIR\nssm.exe" remove "qiperfd" confirm'
    #kill qiperfd
    ${nsProcess::FindProcess} "${QIPERFD_NAME}" $R0
    ${If} $R0 == 0
        DetailPrint "${QIPERFD_NAME} is running. Closing it down"
        ${nsProcess::CloseProcess} "${QIPERFD_NAME}" $R0
        DetailPrint "Waiting for ${QIPERFD_NAME} to close"
        Sleep 2000
    ${Else}
        DetailPrint "${QIPERFD_NAME} was not found to be running"
    ${EndIf}
    ${nsProcess::Unload}
    #kill qiperftray
    ${nsProcess::FindProcess} "${QIPERFTRAY_NAME}" $R0
    ${If} $R0 == 0
        DetailPrint "${QIPERFTRAY_NAME} is running. Closing it down"
        ${nsProcess::CloseProcess} "${QIPERFTRAY_NAME}" $R0
        DetailPrint "Waiting for ${QIPERFTRAY_NAME} to close"
        Sleep 2000
    ${Else}
        DetailPrint "${QIPERFTRAY_NAME} was not found to be running"
    ${EndIf}
    ${nsProcess::Unload}
    #kill nssm.exe
    ${nsProcess::FindProcess} "${SERVICE_WRAPPER}" $R0
    ${If} $R0 == 0
        DetailPrint "${SERVICE_WRAPPER} is running. Closing it down"
        ${nsProcess::CloseProcess} "${SERVICE_WRAPPER}" $R0
        DetailPrint "Waiting for ${SERVICE_WRAPPER} to close"
        Sleep 2000
    ${Else}
        DetailPrint "${SERVICE_WRAPPER} was not found to be running"
    ${EndIf}
    ${nsProcess::Unload}

    !ifdef WIN64
            SetRegView 64
    !endif
    ; Remove startup run
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Run\${QIPERFTRAY_NAME}"
    ; Remove an application from the firewall exception list
    SimpleFC::RemoveApplication "$INSTDIR\${QIPERFD_NAME}"
    Pop $0 ; return error(1)/success(0)
    ; Remove iperf3 from the firewall
    !ifdef WIN64
    SimpleFC::RemoveApplication "$INSTDIR\x86_64\iperf3.exe"
    !else
    SimpleFC::RemoveApplication "$INSTDIR\x86\iperf3.exe"
    !endif
    Pop $0 ; return error(1)/success(0)
    ; Remove iperf2.1 from the firewall
    SimpleFC::RemoveApplication "$INSTDIR\x86\iperf2.1.exe"
    Pop $0 ; return error(1)/success(0)
    ; Remove iperf2 from the firewall
    SimpleFC::RemoveApplication "$INSTDIR\x86\iperf2.exe"
    Pop $0 ; return error(1)/success(0)

FunctionEnd

Function un.install_qiperfc
    #kill qiperfc
    ${nsProcess::FindProcess} "${QIPERFC_NAME}" $R0
    ${If} $R0 == 0
        DetailPrint "${QIPERFC_NAME} is running. Closing it down"
        ${nsProcess::CloseProcess} "${QIPERFC_NAME}" $R0
        DetailPrint "Waiting for ${QIPERFC_NAME} to close"
        Sleep 2000
    ${Else}
        DetailPrint "${QIPERFC_NAME} was not found to be running"
    ${EndIf}
    ${nsProcess::Unload}
    ; Remove an application from the firewall exception list
    SimpleFC::RemoveApplication "$INSTDIR\${QIPERFC_NAME}"
    Pop $0 ; return error(1)/success(0)

FunctionEnd

Function .oninstsuccess
    # final install success, run qiperftray
    SetOutPath "$INSTDIR\"
    Exec "$INSTDIR\${QIPERFTRAY_NAME}"
FunctionEnd

; eof
