; Script generated with the Venis Install Wizard

!addplugindir "nsis\"

; Define your application name
!define APPNAME "qiperfd"
!ifndef APPVERSION
!define APPVERSION 0.8
!endif
!ifndef APPFileVersion
!define APPFileVersion 0.8.11403.10
!endif
!define APPDOMAIN "coolshou.idv.tw"
!define APPURL "https://github.com/coolshou/qiperf"
#!define WIN64 ; force  64 bit, comment out for 32 bit
!define QIPERFD_NAME  "qiperfd.exe"
!define QIPERFC_NAME  "qiperfc.exe"
!define QIPERFTRAY_NAME  "qiperftray.exe"
!define SERVICE_WRAPPER "nssm.exe"

!define PRODUCT_REG_KEY "Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
!define PRODUCT_UNINSTALL_EXE "${APPNAME}-uninstall.exe"

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

!define APPNAMEANDVERSION "qiperf daemon ${APPVERSION}"

; Main Install settings
Name "${APPNAMEANDVERSION}"
InstallDir "$PROGRAMFILES\qiperf"
InstallDirRegKey HKLM "Software\${APPNAME}" ""

!ifdef WIN64
    OutFile "..\${APPNAME}-setup-${APPFileVersion}.exe"
!else
    OutFile "..\${APPNAME}-setup-${APPFileVersion}_x86.exe"
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
!define MUI_ICON "images\${APPNAME}.ico"
# uninstall icon
!define MUI_UNICON "images\uninstall.ico"
!insertmacro MUI_PAGE_WELCOME
Page custom uninstallold ;Custom page
#!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; Set languages (first is default language)
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_RESERVEFILE_LANGDLL

VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "${APPNAMEANDVERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${APPVERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "Comments" "Quick iperf daemon"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "${APPDOMAIN}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalTrademarks" "${APPNAME} is a trademark of ${APPDOMAIN}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "(C)2023-2025 ${APPDOMAIN}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "${APPNAME}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${APPFileVersion}"

Function uninstallold
    # uninstall old version
    ${If} $OLD_VERSION != ""
        ExecWait "$OLD_VERSION"
    ${EndIf}
FunctionEnd

Section "qiperf daemon" SECTION_Daemon
    call kill_process
    ; Set Section properties
    SetOverwrite on

    ; Set Section Files and Shortcuts
    SetOutPath "$INSTDIR\"
    File "images\${APPNAME}.ico"
!ifdef WIN64
    !cd "qiperfd_x86_64"
!else
    !cd "qiperfd_x86"
!endif
!ifdef WIN64
    File "vc_redist.x64.exe"
!else
    File "vc_redist.x86.exe"
!endif
    File "${QIPERFD_NAME}"
    File "D3Dcompiler_47.dll"
    File "opengl32sw.dll"
    File "..\lib\qssh\botan\botan.dll"
    File "Qt6Core${DEBUGSTR}.dll"
    File "Qt6Gui${DEBUGSTR}.dll"
    File "Qt6Network${DEBUGSTR}.dll"
    File "Qt6SerialPort${DEBUGSTR}.dll"
    File "Qt6Svg${DEBUGSTR}.dll"
    File "Qt6WebSockets${DEBUGSTR}.dll"
    File "Qt6Widgets${DEBUGSTR}.dll"
    SetOutPath "$INSTDIR\generic\"
    File "generic\qtuiotouchplugin${DEBUGSTR}.dll"
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
    SetOutPath "$INSTDIR\networkinformation\"
    File "networkinformation\qnetworklistmanager${DEBUGSTR}.dll"
    SetOutPath "$INSTDIR\platforms\"
    File "platforms\qwindows${DEBUGSTR}.dll"
    SetOutPath "$INSTDIR\styles\"
    File "styles\qmodernwindowsstyle${DEBUGSTR}.dll"
    SetOutPath "$INSTDIR\tls"
    File "tls\qcertonlybackend${DEBUGSTR}.dll"
    File "tls\qschannelbackend${DEBUGSTR}.dll"
    SetOutPath "$INSTDIR\translations\"
    ;File "translations\qt_ar.qm"
    ;File "translations\qt_bg.qm"
    ;File "translations\qt_ca.qm"
    ;File "translations\qt_cs.qm"
    ;File "translations\qt_da.qm"
    ;File "translations\qt_de.qm"
    File "translations\qt_en.qm"
    ;File "translations\qt_es.qm"
    ;File "translations\qt_fa.qm"
    ;File "translations\qt_fi.qm"
    ;File "translations\qt_fr.qm"
    ;File "translations\qt_gd.qm"
    ;File "translations\qt_he.qm"
    ;File "translations\qt_hu.qm"
    ;File "translations\qt_it.qm"
    ;File "translations\qt_ja.qm"
    ;File "translations\qt_ka.qm"
    ;File "translations\qt_ko.qm"
    ;File "translations\qt_lg.qm"
    ;File "translations\qt_lv.qm"
    ;File "translations\qt_nl.qm"
    ;File "translations\qt_nn.qm"
    ;File "translations\qt_pl.qm"
    ;File "translations\qt_pt_BR.qm"
    ;File "translations\qt_ru.qm"
    ;File "translations\qt_sk.qm"
    ;File "translations\qt_tr.qm"
    ;File "translations\qt_uk.qm"
    File "translations\qt_zh_CN.qm"
    File "translations\qt_zh_TW.qm"
    ;SetOutPath "$INSTDIR\windows\x86\"
    ;File "windows\x86\cygcrypto-1.1.dll"
    ;File "windows\x86\cyggcc_s-1.dll"
    ;File "windows\x86\cygwin1.dll"
    ;File "windows\x86\cygz.dll"
    File "windows\x86\iperf2.exe"
    ;File "windows\x86\iperf2.1.exe"
    ;File "windows\x86\iperf2.2.n.exe"
    ;File "windows\x86\iperf3.exe"
    SetOutPath "$INSTDIR\windows\x86_64\"
    File "windows\x86_64\cygcrypto-1.1.dll"
    File "windows\x86_64\cygwin1.dll"
    File "windows\x86_64\cygz.dll"
    File "windows\x86_64\iperf2.1.exe"
    File "windows\x86_64\iperf2.2.n.exe"
    File "windows\x86_64\iperf3.exe"
    SetOutPath "$INSTDIR\"
    !cd ..

!ifdef WIN64
    !cd qiperftray_x86_64
!else
    !cd qiperftray_x86
!endif
    File "dxcompiler.dll"
    File "dxil.dll"
    File "${QIPERFTRAY_NAME}"
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
    CreateShortCut "$SMPROGRAMS\qiperf\qiperfd-Uninstall.lnk" "$INSTDIR\${PRODUCT_UNINSTALL_EXE}"

    WriteRegStr HKLM "Software\${PRODUCT_REG_KEY}" "InstallMode" "0"
    # set QIPERFTRAY_NAME run on system boot
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "${QIPERFTRAY_NAME}" '"$INSTDIR\${QIPERFTRAY_NAME}"'

    Call check_vc_redist
    Call install_qiperfd
SectionEnd


Section -FinishSection
    WriteRegStr HKLM "Software\${APPNAME}" "" "$INSTDIR"
    WriteRegStr HKLM "Software\${PRODUCT_REG_KEY}" "DisplayName" "${APPNAMEANDVERSION}"
    WriteRegStr HKLM "Software\${PRODUCT_REG_KEY}" "DisplayIcon" "$INSTDIR\${APPNAME}.ico"
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
;!insertmacro MUI_FUNCTION_DESCRIPTION_END

;Uninstall section
Section Uninstall
    Call un.install_qiperfd
!ifdef WIN64
    SetRegView 64
!endif
    ;Remove from registry...
    DeleteRegKey HKLM "Software\${PRODUCT_REG_KEY}"
    DeleteRegKey HKLM "SOFTWARE\${APPNAME}"

    ; Delete self
    Delete "$INSTDIR\${PRODUCT_UNINSTALL_EXE}"
    Delete "$INSTDIR\${APPNAME}.ico"
    ; Delete Shortcuts
    #Delete "$DESKTOP\qiperftray.lnk"
    Delete "$SMPROGRAMS\qiperf\qiperfd.lnk"
    Delete "$SMPROGRAMS\qiperf\qiperftray.lnk"
    Delete "$SMPROGRAMS\qiperf\qiperfd-Uninstall.lnk"

    ; Clean up qiperf daemon
!ifdef WIN64
    Delete "$INSTDIR\vc_redist.x64.exe"
!else
    Delete "$INSTDIR\vc_redist.x86.exe"
!endif
    Delete "$INSTDIR\dxcompiler.dll"
    Delete "$INSTDIR\dxil.dll"
    Delete "$INSTDIR\${QIPERFD_NAME}"
    Delete "$INSTDIR\Qt6Core${DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6Gui${DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6Network${DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6Svg{DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6SerialPort${DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6WebSockets${DEBUGSTR}.dll"
    Delete "$INSTDIR\Qt6Widgets${DEBUGSTR}.dll"
    Delete "$INSTDIR\botan.dll"
    Delete "$INSTDIR\generic\qtuiotouchplugin${DEBUGSTR}.dll"
    Delete "$INSTDIR\networkinformation\qnetworklistmanager${DEBUGSTR}.dll"
    Delete "$INSTDIR\tls\qcertonlybackend${DEBUGSTR}.dll"
    Delete "$INSTDIR\tls\qschannelbackend${DEBUGSTR}.dll"

    Delete "$INSTDIR\styles\qmodernwindowsstyle${DEBUGSTR}.dll"
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

    ;Delete "$INSTDIR\translations\qt_ar.qm"
    ;Delete "$INSTDIR\translations\qt_bg.qm"
    ;Delete "$INSTDIR\translations\qt_ca.qm"
    ;Delete "$INSTDIR\translations\qt_cs.qm"
    ;Delete "$INSTDIR\translations\qt_da.qm"
    ;Delete "$INSTDIR\translations\qt_de.qm"
    Delete "$INSTDIR\translations\qt_en.qm"
    ;Delete "$INSTDIR\translations\qt_es.qm"
    ;Delete "$INSTDIR\translations\qt_fa.qm"
    ;Delete "$INSTDIR\translations\qt_fi.qm"
    ;Delete "$INSTDIR\translations\qt_fr.qm"
    ;Delete "$INSTDIR\translations\qt_gd.qm"
    ;Delete "$INSTDIR\translations\qt_he.qm"
    ;Delete "$INSTDIR\translations\qt_hu.qm"
    ;Delete "$INSTDIR\translations\qt_it.qm"
    ;Delete "$INSTDIR\translations\qt_ja.qm"
    ;Delete "$INSTDIR\translations\qt_ka.qm"
    ;Delete "$INSTDIR\translations\qt_ko.qm"
    ;Delete "$INSTDIR\translations\qt_lg.qm"
    ;Delete "$INSTDIR\translations\qt_lv.qm"
    ;Delete "$INSTDIR\translations\qt_nl.qm"
    ;Delete "$INSTDIR\translations\qt_nn.qm"
    ;Delete "$INSTDIR\translations\qt_pl.qm"
    ;Delete "$INSTDIR\translations\qt_pt_BR.qm"
    ;Delete "$INSTDIR\translations\qt_ru.qm"
    ;Delete "$INSTDIR\translations\qt_sk.qm"
    ;Delete "$INSTDIR\translations\qt_tr.qm"
    ;Delete "$INSTDIR\translations\qt_uk.qm"
    Delete "$INSTDIR\translations\qt_zh_CN.qm"
    Delete "$INSTDIR\translations\qt_zh_TW.qm"

    ;Delete "$INSTDIR\windows\x86\cygcrypto-1.1.dll"
    ;Delete "$INSTDIR\windows\x86\cyggcc_s-1.dll"
    ;Delete "$INSTDIR\windows\x86\cygwin1.dll"
    ;Delete "$INSTDIR\windows\x86\cygz.dll"
    Delete "$INSTDIR\windows\x86\iperf2.exe"
    ;Delete "$INSTDIR\windows\x86\iperf2.1.exe"
    ;Delete "$INSTDIR\windows\x86\iperf2.2.n.exe"
    ;Delete "$INSTDIR\windows\x86\iperf3.exe"
    Delete "$INSTDIR\windows\x86_64\cygcrypto-1.1.dll"
    Delete "$INSTDIR\windows\x86_64\cygwin1.dll"
    Delete "$INSTDIR\windows\x86_64\cygz.dll"
    Delete "$INSTDIR\windows\x86_64\iperf2.1.exe"
    Delete "$INSTDIR\windows\x86_64\iperf2.2.n.exe"
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

    ; Remove remaining directories
    RMDir "$SMPROGRAMS\qiperf"
    RMDir "$INSTDIR\windows\x86_64\"
    ;RMDir "$INSTDIR\windows\x86\"
    RMDir "$INSTDIR\windows\"
    RMDir "$INSTDIR\networkinformation\"
    RMDir "$INSTDIR\tls\"
    RMDir "$INSTDIR\generic\"
    RMDir "$INSTDIR\resources\"
    RMDir "$INSTDIR\translations\"
    RMDir "$INSTDIR\styles\"
    RMDir "$INSTDIR\platforms\"
    RMDir "$INSTDIR\imageformats\"
    RMDir "$INSTDIR\iconengines\"
    RMDir "$INSTDIR\"

SectionEnd

BrandingText "Quick iperf daemon"

Function .onInit
    ; use command line parameters /S for Silent mode

    ${If} ${RunningX64}
    !ifdef WIN64
        SetRegView 64
    !endif
    ${Else}
    !ifdef WIN64
        MessageBox MB_OK|MB_ICONSTOP 'This is the 64 bit ${APPNAME} installer$\r$\nPlease download the 32 bit version $\r$\nClick Ok to quit Setup.' /SD IDOK
        Quit
    !endif
    ${EndIf}

# ;Check earlier installation
    DetailPrint "Detect old version of qiperf daemon..."
    ClearErrors
    ReadRegStr $0 HKLM "Software\${PRODUCT_REG_KEY}" "DisplayVersion"
    IfErrors init.uninst ; older versions might not have "Version" string set
    ${VersionCompare} $0 ${APPFileVersion} $1
    IntCmp $1 2 init.uninst
    MessageBox MB_YESNO|MB_ICONQUESTION "${APPNAME} version $0 seems to be already installed on your system.$\nWould you like to proceed with the installation of version ${APPFileVersion}?" /SD IDYES IDYES init.uninst
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
      strcpy $INSTDIR "$PROGRAMFILES64\qiperf"
    !endif

    # set section 'daemon' as selected and read-only
    #;IntOp $0 ${SF_SELECTED} | ${SF_RO}
    #;SectionSetFlags ${SECTION_Daemon} $0

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
    ; Add iperf2.2.n to firewall
    !ifdef WIN64
    SimpleFC::AddApplication "iperf2.2.n" "$INSTDIR\x86_64\iperf2.2.n.exe" 0 2 "" 1
    !else
    SimpleFC::AddApplication "iperf2.2.n" "$INSTDIR\x86\iperf2.2.n.exe" 0 2 "" 1
    !endif
    Pop $0 ; return error(1)/success(0)
    ; Add iperf2.1 to firewall
    ;SimpleFC::AddApplication "iperf2.1" "$INSTDIR\x86\iperf2.1.exe" 0 2 "" 1
    ;Pop $0 ; return error(1)/success(0)
    ; Add iperf2 to firewall
    ;SimpleFC::AddApplication "iperf2" "$INSTDIR\x86\iperf2.exe" 0 2 "" 1
    ;Pop $0 ; return error(1)/success(0)

    # install qiperfd  service & start it
    ExecWait '"$INSTDIR\nssm.exe" install "qiperfd" "$INSTDIR\${QIPERFD_NAME}"'
    ExecWait '"$INSTDIR\nssm.exe" start "qiperfd"'
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

!macro kill_process un
Function ${un}kill_process
    # uninstall qiperfd  service
    ExecWait '"$INSTDIR\nssm.exe" stop "qiperfd"'

    #kill qiperfd
    ${nsProcess::FindProcess} "${QIPERFD_NAME}" $R0
    ${If} $R0 == 0
        DetailPrint "${QIPERFD_NAME} is running. Closing it down"
        ${nsProcess::KillProcess} "${QIPERFD_NAME}" $R0
        ;DetailPrint "Waiting for ${QIPERFD_NAME} to close"
        ;Sleep 2000
    ${Else}
        DetailPrint "${QIPERFD_NAME} was not found to be running"
    ${EndIf}
    ${nsProcess::Unload}
    #kill qiperftray
    ${nsProcess::FindProcess} "${QIPERFTRAY_NAME}" $R0
    ${If} $R0 == 0
        DetailPrint "${QIPERFTRAY_NAME} is running. Closing it down"
        ${nsProcess::KillProcess} "${QIPERFTRAY_NAME}" $R0
        ;DetailPrint "Waiting for ${QIPERFTRAY_NAME} to close"
        ;Sleep 2000
    ${Else}
        DetailPrint "${QIPERFTRAY_NAME} was not found to be running"
    ${EndIf}
    ${nsProcess::Unload}
    #kill nssm.exe
    ${nsProcess::FindProcess} "${SERVICE_WRAPPER}" $R0
    ${If} $R0 == 0
        DetailPrint "${SERVICE_WRAPPER} is running. Closing it down"
        ${nsProcess::KillProcess} "${SERVICE_WRAPPER}" $R0
        ;DetailPrint "Waiting for ${SERVICE_WRAPPER} to close"
        ;Sleep 2000
    ${Else}
        DetailPrint "${SERVICE_WRAPPER} was not found to be running"
    ${EndIf}
    ${nsProcess::Unload}
FunctionEnd
!macroend
!insertmacro kill_process ""
!insertmacro kill_process "un."

Function un.install_qiperfd
    call un.kill_process
    # uninstall qiperfd  service
    #ExecWait '"$INSTDIR\nssm.exe" stop "qiperfd"'
    ExecWait '"$INSTDIR\nssm.exe" remove "qiperfd" confirm'

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
    ; Remove iperf2.2.n from the firewall
    !ifdef WIN64
    SimpleFC::RemoveApplication "$INSTDIR\x86_64\iperf2.2.n.exe"
    !else
    SimpleFC::RemoveApplication "$INSTDIR\x86\iperf2.2.n.exe"
    !endif
    Pop $0 ; return error(1)/success(0)

    ; Remove iperf2.1 from the firewall
    ;SimpleFC::RemoveApplication "$INSTDIR\x86\iperf2.1.exe"
    ;Pop $0 ; return error(1)/success(0)
    ; Remove iperf2 from the firewall
    ;SimpleFC::RemoveApplication "$INSTDIR\x86\iperf2.exe"
    ;Pop $0 ; return error(1)/success(0)

FunctionEnd

Function .oninstsuccess
    # final install success, run qiperftray
    SetOutPath "$INSTDIR\"
    Exec "$INSTDIR\${QIPERFTRAY_NAME}"
FunctionEnd

; eof
