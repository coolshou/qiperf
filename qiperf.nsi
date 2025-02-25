; Script generated with the Venis Install Wizard

!addplugindir "nsis\"

; Define your application name
!define APPNAME "qiperf"
!ifndef APPVERSION
!define APPVERSION 0.8
!endif
!ifndef APPFileVersion
!define APPFileVersion 0.8.11402.24
!endif
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
!ifdef WIN64
    File "..\qiperfd-setup-${APPFileVersion}.exe"
!else
    File "..\qiperfd-setup-${APPFileVersion}_x86.exe"
!endif

    ;Call check_vc_redist
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
    SetOutPath "$INSTDIR\printsupport\"
    File "printsupport\windowsprintersupport.dll"
!endif
    SetOutPath "$INSTDIR\template\"
    File "..\qiperfc\template\result.html"

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
;!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
;    !insertmacro MUI_DESCRIPTION_TEXT ${SECTION_Daemon} "quick iperf daemon && systray"
;    !insertmacro MUI_DESCRIPTION_TEXT ${SECTION_Console} "quick iperf console"
;!insertmacro MUI_FUNCTION_DESCRIPTION_END

;Uninstall section
Section Uninstall
    ;Call un.install_qiperfd
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
    Delete "$DESKTOP\qiperfc.lnk"
    Delete "$SMPROGRAMS\qiperf\qiperfc.lnk"
    Delete "$SMPROGRAMS\qiperf\Uninstall.lnk"

    ; Clean up qiperf daemon
!ifdef WIN64
    Delete "$INSTDIR\qiperfd-setup-${APPFileVersion}.exe"
!else
    Delete "$INSTDIR\qiperfd-setup-${APPFileVersion}_x86.exe"
!endif
!ifdef QT6
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

    ; Clean up qiperf console
    Delete "$INSTDIR\${QIPERFC_NAME}"
    Delete "$INSTDIR\template\result.html"

    ; Remove remaining directories
    RMDir "$SMPROGRAMS\qiperf"
!ifdef QT6
    RMDir "$INSTDIR\platforminputcontexts\"
    RMDir "$INSTDIR\position\"
    RMDir "$INSTDIR\qml\"
    RMDir "$INSTDIR\qmltooling\"
    RMDir "$INSTDIR\translations\qtwebengine_locales\"
!endif
    RMDir "$INSTDIR\printsupport\"
    RMDir "$INSTDIR\template"
    RMDir "$INSTDIR\"

    ${unregisterExtension} ".qip" "Quick Iperf config File"

SectionEnd

BrandingText "Quick iperf"

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
    DetailPrint "Detect old version of qiperf..."
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

init.done:
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
   #${If} $OLD_INSTALL_MODE  == "1"
   #    SectionSetFlags ${SECTION_Console}  ${SF_SELECTED}
   #${Else}
   #   SectionSetFlags ${SECTION_Console} 0
   #${EndIf}
FunctionEnd

Function install_qiperfd
    !ifdef WIN64
        ExecWait '$INSTDIR\qiperfd-setup-${APPFileVersion}.exe /s'
    !else
        ExecWait '$INSTDIR\qiperfd-setup-${APPFileVersion}_x86.exe /s'
    !endif
FunctionEnd

Function install_vc_redist
    DetailPrint "Install VC runtime for qiperf..."
    ;not installed, so run the installer in quiet mode
    !ifdef WIN64
    ExecWait '$INSTDIR\vc_redist.x64.exe /q /norestart'
    !else
    ExecWait '$INSTDIR\vc_redist.x86.exe /q /norestart'
    !endif
FunctionEnd

Function check_vc_redist
    DetailPrint "Check VC runtime..."
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
    ExecWait '"$INSTDIR\nssm.exe" stop "qiperfd"'
    ExecWait '"$INSTDIR\nssm.exe" remove "qiperfd" confirm'
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

#Function .oninstsuccess
    # final install success, run qiperftray
 #   SetOutPath "$INSTDIR\"
 #   Exec "$INSTDIR\${QIPERFTRAY_NAME}"
#FunctionEnd

; eof
