; Script generated with the Venis Install Wizard

; Define your application name
!define APPNAME "qiperf"
!define APPVERSION 0.6
!define APPFileVersion 0.6.11310.04
!define APPDOMAIN "coolshou.idv.tw"
!define APPURL "https://github.com/coolshou/qiperf"
#!define WIN64 ; force  64 bit, comment out for 32 bit
!define QIPERFD_NAME  "qiperfd.exe"
!define QIPERFC_NAME  "qiperfc.exe"
!define QIPERFTRAY_NAME  "qiperftray.exe"
!define SERVICE_WRAPPER "nssm.exe"

!define PRODUCT_REG_KEY "Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
!define PRODUCT_UNINSTALL_EXE "uninstall.exe"

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
    OutFile "qiperf-setup-${APPFileVersion}.exe"
!else
    OutFile "qiperf-setup-${APPFileVersion}_x86.exe"
!endif

!include "FileFunc.nsh"
; Use compression
SetCompressor LZMA
!include "x64.nsh"
; Modern interface settings
!include "MUI.nsh"
!include "nsProcess.nsh"
!include "FileAssociation.nsh"
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
    File "libgcc_s_seh-1.dll"
    File "libstdc++-6.dll"
    File "libwinpthread-1.dll"
    File "${QIPERFD_NAME}"
    File "Qt5Core.dll"
    File "Qt5Network.dll"
    File "Qt5WebSockets.dll"
    SetOutPath "$INSTDIR\bearer\"
    File "bearer\qgenericbearer.dll"
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
    File "windows\x86\iperf21.exe"
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
    File "libEGL.dll"
    File "libGLESv2.dll"
    File "opengl32sw.dll"
    File "${QIPERFTRAY_NAME}"
    File "Qt5Gui.dll"
    File "Qt5Svg.dll"
    File "Qt5Widgets.dll"
    SetOutPath "$INSTDIR\iconengines\"
    File "iconengines\qsvgicon.dll"
    SetOutPath "$INSTDIR\imageformats\"
    File "imageformats\qgif.dll"
    File "imageformats\qicns.dll"
    File "imageformats\qico.dll"
    File "imageformats\qjpeg.dll"
    File "imageformats\qsvg.dll"
    File "imageformats\qtga.dll"
    File "imageformats\qtiff.dll"
    File "imageformats\qwbmp.dll"
    File "imageformats\qwebp.dll"
    SetOutPath "$INSTDIR\platforms\"
    File "platforms\qwindows.dll"
    SetOutPath "$INSTDIR\styles\"
    File "styles\qwindowsvistastyle.dll"
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
    File "Qt5PrintSupport.dll"
    SetOutPath "$INSTDIR\template\"
    File "..\qiperfc\template\result.html"
    SetOutPath "$INSTDIR\printsupport\"
    File "printsupport\windowsprintersupport.dll"
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
    Delete "$INSTDIR\libgcc_s_seh-1.dll"
    Delete "$INSTDIR\libstdc++-6.dll"
    Delete "$INSTDIR\libwinpthread-1.dll"
    Delete "$INSTDIR\${QIPERFD_NAME}"
    Delete "$INSTDIR\Qt5Core.dll"
    Delete "$INSTDIR\Qt5Network.dll"
    Delete "$INSTDIR\Qt5WebSockets.dll"
    Delete "$INSTDIR\bearer\qgenericbearer.dll"
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
    Delete "$INSTDIR\windows\x86\iperf21.exe"
    Delete "$INSTDIR\windows\x86\iperf3.exe"
    Delete "$INSTDIR\windows\x86_64\cygcrypto-1.1.dll"
    Delete "$INSTDIR\windows\x86_64\cygwin1.dll"
    Delete "$INSTDIR\windows\x86_64\cygz.dll"
    Delete "$INSTDIR\windows\x86_64\iperf3.exe"

    Delete "$INSTDIR\D3Dcompiler_47.dll"
    Delete "$INSTDIR\libEGL.dll"
    Delete "$INSTDIR\libGLESv2.dll"
    Delete "$INSTDIR\opengl32sw.dll"
    Delete "$INSTDIR\${QIPERFTRAY_NAME}"
    Delete "$INSTDIR\nssm.exe"
    Delete "$INSTDIR\Qt5Gui.dll"
    Delete "$INSTDIR\Qt5Svg.dll"
    Delete "$INSTDIR\Qt5Widgets.dll"
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
    Delete "$INSTDIR\styles\qwindowsvistastyle.dll"

    ; Clean up qiperf console
    Delete "$INSTDIR\${QIPERFC_NAME}"
    Delete "$INSTDIR\Qt5PrintSupport.dll"
    Delete "$INSTDIR\printsupport\windowsprintersupport.dll"
    Delete "$INSTDIR\template\result.html"

    ; Remove remaining directories
    RMDir "$SMPROGRAMS\qiperf"
    RMDir "$INSTDIR\windows\x86_64\"
    RMDir "$INSTDIR\windows\x86\"
    RMDir "$INSTDIR\windows\"
    RMDir "$INSTDIR\translations\"
    RMDir "$INSTDIR\styles\"
    RMDir "$INSTDIR\printsupport\"
    RMDir "$INSTDIR\platforms\"
    RMDir "$INSTDIR\imageformats\"
    RMDir "$INSTDIR\iconengines\"
    RMDir "$INSTDIR\bearer\"
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

    # get preview install mode
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

    # install qiperfd  service & start it
    Exec '"$INSTDIR\nssm.exe" install "qiperfd" "$INSTDIR\${QIPERFD_NAME}"'
    Exec '"$INSTDIR\nssm.exe" start "qiperfd"'
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
