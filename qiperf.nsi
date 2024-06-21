; Script generated with the Venis Install Wizard

; Define your application name
!define APPNAME "qiperf"
!define APPVERSION 0.2.0.0
!define APPFileVersion 0.2.11306.21
!define APPDOMAIN "coolshou.idv.tw"
!define APPURL "https://github.com/coolshou/qiperf"
#!define WIN64 ; comment out for 32 bit
!define QIPERFD_NAME  "qiperfd.exe"
!define QIPERFC_NAME  "qiperfc.exe"
!define QIPERFTRAY_NAME  "qiperftray.exe"

!define PRODUCT_REG_KEY "Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"

VIProductVersion ${APPFileVersion}

!define APPNAMEANDVERSION "qiperf ${APPVERSION}"

; Main Install settings
Name "${APPNAMEANDVERSION}"
InstallDir "$PROGRAMFILES\qiperf"
InstallDirRegKey HKLM "Software\${APPNAME}" ""

OutFile "qiperf-setup-${APPFileVersion}.exe"

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
Var UNINSTALL_OLD_VERSION

!define MUI_ABORTWARNING
!define MUI_ICON "images\qiperf.ico"
# uninstall icon
!define MUI_UNICON "images\uninstall.ico"
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_FUNCTION "StartQIperfd"
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
        CreateShortCut "$DESKTOP\qiperftray.lnk" "$INSTDIR\${QIPERFTRAY_NAME}"

        CreateDirectory "$SMPROGRAMS\qiperf"
        #CreateShortCut "$SMPROGRAMS\qiperf\qiperfd.lnk" "$INSTDIR\${QIPERFD_NAME}"
        CreateShortCut "$SMPROGRAMS\qiperf\qiperftray.lnk" "$INSTDIR\${QIPERFTRAY_NAME}"
        CreateShortCut "$SMPROGRAMS\qiperf\Uninstall.lnk" "$INSTDIR\uninstall.exe"

        Call install_qiperfd


SectionEnd

Section "qiperf console" SECTION_Console
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
        SetOutPath "$INSTDIR\printsupport\"
        File "printsupport\windowsprintersupport.dll"
        !cd ..
        CreateShortCut "$DESKTOP\qiperfc.lnk" "$INSTDIR\${QIPERFC_NAME}"
        CreateShortCut "$SMPROGRAMS\qiperf\qiperfc.lnk" "$INSTDIR\${QIPERFC_NAME}"

        ${registerExtension} "$INSTDIR\${QIPERFC_NAME}" ".qip" "Quick Iperf config File"

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
        WriteRegStr HKLM "Software\${PRODUCT_REG_KEY}" "UninstallString" "$INSTDIR\uninstall.exe"
        WriteUninstaller "$INSTDIR\uninstall.exe"
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
    !ifdef WIN64
            SetRegView 64
    !endif
        ;Remove from registry...
        DeleteRegKey HKLM "Software\${PRODUCT_REG_KEY}"
        DeleteRegKey HKLM "SOFTWARE\${APPNAME}"

        ; Delete self
        Delete "$INSTDIR\uninstall.exe"
        Delete "$INSTDIR\qiperf.ico"
        ; Delete Shortcuts
        Delete "$DESKTOP\qiperftray.lnk"
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
        RMDir "$INSTDIR\"

        ${unregisterExtension} ".qip" "Quick Iperf config File"

SectionEnd

BrandingText "Quick iperf daemon"

Function .onInit
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
  ReadRegStr $0 HKLM "Software\${PRODUCT_REG_KEY}" ""
  IfErrors init.done
  StrCpy $UNINSTALL_OLD_VERSION '"$0\uninstall.exe" /S _?=$0'
  ExecWait '$UNINSTALL_OLD_VERSION'

init.done:

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
    !ifdef WIN64
      strcpy $INSTDIR "$PROGRAMFILES64\${APPNAME}"
    !endif

  # set section 'daemon' as selected and read-only
  IntOp $0 ${SF_SELECTED} | ${SF_RO}
  SectionSetFlags ${SECTION_Daemon} $0
        # set section 'console' as unselected
        #IntOp $0 ~${SF_SELECTED}
        SectionSetFlags ${SECTION_Console} 0
FunctionEnd

Function install_qiperfd
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "qiperfd" '"$INSTDIR\${QIPERFD_NAME}"'
  # start

  # install qiperfd as service  => require app implement SERVICE API
  #SimpleSC::InstallService "qiperfd" "quick iperf daemon" "16" "2" "$INSTDIR\qiperfd.exe" "" "" ""
  #Pop $0 ; returns an errorcode (<>0) otherwise success (0)
  #SimpleSC::StartService "qiperfd" "" "100"
FunctionEnd

Function un.install_qiperfd
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
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Run\qiperfd"
    #kill qiperfc

  # uninstall qiperfd as service
  #SimpleSC::StopService "qiperfd" "1" "60"
  #Pop $0 ; returns an errorcode (<>0) otherwise success (0)
  #SimpleSC::RemoveService "qiperfd"
  #Pop $0 ; returns an errorcode (<>0) otherwise success (0)
  #DeleteRegKey HKLM "SYSTEM\CurrentControlSet\Services\qiperfd"

FunctionEnd

Function StartQIperfd
    # run the QIPERFD
    SetOutPath "$INSTDIR\"
    #    nsExec::Exec "$INSTDIR\${QIPERFD_NAME}"
    #    #Pop $ExitCode
    #    # run the QIPERFTRAY
    #    nsExec::Exec "$INSTDIR\${QIPERFTRAY_NAME}"
     #   #Pop $ExitCode
    ExecShell "" "$INSTDIR\${QIPERFD_NAME}" SW_HIDE
    Exec "$INSTDIR\${QIPERFTRAY_NAME}"

FunctionEnd

; eof
