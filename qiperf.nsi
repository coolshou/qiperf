; Script generated with the Venis Install Wizard

; Define your application name
!define APPNAME "qiperf"
!define APPVERSION 0.2.0.0
!define APPFileVersion 0.2.11209.20
!define APPDOMAIN "coolshou.idv.tw"
!define APPURL "https://github.com/coolshou/qiperf"
!define WIN64 ; comment out for 32 bit

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

!define MUI_ABORTWARNING
!define MUI_ICON "images\qiperf.ico"
# uninstall icon
!define MUI_UNICON "images\uninstall.ico"
!insertmacro MUI_PAGE_WELCOME
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

Section "qiperf daemon" SECTION_Daemon
	; Set Section properties
	SetOverwrite on

	; Set Section Files and Shortcuts
	SetOutPath "$INSTDIR\"
	File "images\qiperf.ico"
	File "qiperfd_x86_64\libgcc_s_seh-1.dll"
	File "qiperfd_x86_64\libstdc++-6.dll"
	File "qiperfd_x86_64\libwinpthread-1.dll"
	File "qiperfd_x86_64\qiperfd.exe"
	File "qiperfd_x86_64\Qt5Core.dll"
	File "qiperfd_x86_64\Qt5Network.dll"
	File "qiperfd_x86_64\Qt5WebSockets.dll"
	SetOutPath "$INSTDIR\bearer\"
	File "qiperfd_x86_64\bearer\qgenericbearer.dll"
	SetOutPath "$INSTDIR\translations\"
	File "qiperfd_x86_64\translations\qt_ar.qm"
	File "qiperfd_x86_64\translations\qt_bg.qm"
	File "qiperfd_x86_64\translations\qt_ca.qm"
	File "qiperfd_x86_64\translations\qt_cs.qm"
	File "qiperfd_x86_64\translations\qt_da.qm"
	File "qiperfd_x86_64\translations\qt_de.qm"
	File "qiperfd_x86_64\translations\qt_en.qm"
	File "qiperfd_x86_64\translations\qt_es.qm"
	File "qiperfd_x86_64\translations\qt_fi.qm"
	File "qiperfd_x86_64\translations\qt_fr.qm"
	File "qiperfd_x86_64\translations\qt_gd.qm"
	File "qiperfd_x86_64\translations\qt_he.qm"
	File "qiperfd_x86_64\translations\qt_hu.qm"
	File "qiperfd_x86_64\translations\qt_it.qm"
	File "qiperfd_x86_64\translations\qt_ja.qm"
	File "qiperfd_x86_64\translations\qt_ko.qm"
	File "qiperfd_x86_64\translations\qt_lv.qm"
	File "qiperfd_x86_64\translations\qt_pl.qm"
	File "qiperfd_x86_64\translations\qt_ru.qm"
	File "qiperfd_x86_64\translations\qt_sk.qm"
	File "qiperfd_x86_64\translations\qt_tr.qm"
	File "qiperfd_x86_64\translations\qt_uk.qm"
	File "qiperfd_x86_64\translations\qt_zh_TW.qm"
	SetOutPath "$INSTDIR\windows\x86\"
	File "qiperfd_x86_64\windows\x86\cygcrypto-1.1.dll"
	File "qiperfd_x86_64\windows\x86\cyggcc_s-1.dll"
	File "qiperfd_x86_64\windows\x86\cygwin1.dll"
	File "qiperfd_x86_64\windows\x86\cygz.dll"
	File "qiperfd_x86_64\windows\x86\iperf2.exe"
	File "qiperfd_x86_64\windows\x86\iperf21.exe"
	File "qiperfd_x86_64\windows\x86\iperf3.exe"
	SetOutPath "$INSTDIR\windows\x86_64\"
	File "qiperfd_x86_64\windows\x86_64\cygcrypto-1.1.dll"
	File "qiperfd_x86_64\windows\x86_64\cygwin1.dll"
	File "qiperfd_x86_64\windows\x86_64\cygz.dll"
	File "qiperfd_x86_64\windows\x86_64\iperf3.exe"
	SetOutPath "$INSTDIR\"
	File "qiperftray_x86_64\D3Dcompiler_47.dll"
	File "qiperftray_x86_64\libEGL.dll"
	File "qiperftray_x86_64\libGLESv2.dll"
	File "qiperftray_x86_64\opengl32sw.dll"
	File "qiperftray_x86_64\qiperftray.exe"
	File "qiperftray_x86_64\Qt5Gui.dll"
	File "qiperftray_x86_64\Qt5Svg.dll"
	File "qiperftray_x86_64\Qt5Widgets.dll"
	SetOutPath "$INSTDIR\iconengines\"
	File "qiperftray_x86_64\iconengines\qsvgicon.dll"
	SetOutPath "$INSTDIR\imageformats\"
	File "qiperftray_x86_64\imageformats\qgif.dll"
	File "qiperftray_x86_64\imageformats\qicns.dll"
	File "qiperftray_x86_64\imageformats\qico.dll"
	File "qiperftray_x86_64\imageformats\qjpeg.dll"
	File "qiperftray_x86_64\imageformats\qsvg.dll"
	File "qiperftray_x86_64\imageformats\qtga.dll"
	File "qiperftray_x86_64\imageformats\qtiff.dll"
	File "qiperftray_x86_64\imageformats\qwbmp.dll"
	File "qiperftray_x86_64\imageformats\qwebp.dll"
	SetOutPath "$INSTDIR\platforms\"
	File "qiperftray_x86_64\platforms\qwindows.dll"
	SetOutPath "$INSTDIR\styles\"
	File "qiperftray_x86_64\styles\qwindowsvistastyle.dll"
	
	CreateShortCut "$DESKTOP\qiperftray.lnk" "$INSTDIR\qiperftray.exe"
	
	CreateDirectory "$SMPROGRAMS\qiperf"
	#CreateShortCut "$SMPROGRAMS\qiperf\qiperfd.lnk" "$INSTDIR\qiperfd.exe"
	CreateShortCut "$SMPROGRAMS\qiperf\qiperftray.lnk" "$INSTDIR\qiperftray.exe"
	CreateShortCut "$SMPROGRAMS\qiperf\Uninstall.lnk" "$INSTDIR\uninstall.exe"
	
	Call install_qiperfd
	
SectionEnd

Section "qiperf console" SECTION_Console
	; Set Section properties
	SetOverwrite on

	; Set Section Files and Shortcuts
	SetOutPath "$INSTDIR\"
	File "qiperfc_x86_64\qiperfc.exe"
	File "qiperfc_x86_64\Qt5PrintSupport.dll"
	SetOutPath "$INSTDIR\printsupport\"
	File "qiperfc_x86_64\printsupport\windowsprintersupport.dll"
	
	CreateShortCut "$DESKTOP\qiperfc.lnk" "$INSTDIR\qiperfc.exe"
	CreateShortCut "$SMPROGRAMS\qiperf\qiperfc.lnk" "$INSTDIR\qiperfc.exe"
SectionEnd

Section -FinishSection

	WriteRegStr HKLM "Software\${APPNAME}" "" "$INSTDIR"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayName" "${APPNAME}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayIcon" "$INSTDIR\qiperf.ico"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "Publisher" "${APPDOMAIN}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "DisplayVersion" "${APPFileVersion}"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "HelpLink" "${APPURL}"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoModify" "1"
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "NoRepair" "1"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "UninstallString" "$INSTDIR\uninstall.exe"
	WriteUninstaller "$INSTDIR\uninstall.exe"
	# size
	${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
	IntFmt $0 "0x%08X" $0
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}" "EstimatedSize" "$0"
 
SectionEnd

; Modern install component descriptions
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${SECTION_Daemon} "quick iperf daemon && systray"
	!insertmacro MUI_DESCRIPTION_TEXT ${SECTION_Console} "quick iperf console"
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;Uninstall section
Section Uninstall
	Call un.install_qiperfd
	;Remove from registry...
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAME}"
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
	Delete "$INSTDIR\qiperfd.exe"
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
	Delete "$INSTDIR\qiperftray.exe"
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
	Delete "$INSTDIR\qiperfc.exe"
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

SectionEnd

BrandingText "Quick iperf daemon"

Function .onInit
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
  # install qiperfd as service
	SimpleSC::InstallService "qiperfd" "quick iperf daemon" "16" "2" "$INSTDIR\qiperfd.exe" "" "" ""
  Pop $0 ; returns an errorcode (<>0) otherwise success (0)
  SimpleSC::StartService "qiperfd" "" "100"
FunctionEnd

Function un.install_qiperfd
  # uninstall qiperfd as service
	SimpleSC::StopService "qiperfd" "1" "60"
	Pop $0 ; returns an errorcode (<>0) otherwise success (0)
  SimpleSC::RemoveService "qiperfd"
	Pop $0 ; returns an errorcode (<>0) otherwise success (0)
  #DeleteRegKey HKLM "SYSTEM\CurrentControlSet\Services\qiperfd"
  
FunctionEnd

; eof