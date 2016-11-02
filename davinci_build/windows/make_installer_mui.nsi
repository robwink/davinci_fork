!include LogicLib.nsh

!include "MUI2.nsh"

; --------------------------------

; Change these as needed
!define VERSION "2.18"
!define INST_FOLDER "davinci_win"


Name "Davinci-${VERSION}"
OutFile "Davinci-${VERSION}-Setup.exe"

; Default install folder
InstallDir "$PROGRAMfILES64\Davinci-${VERSION}"

; Do registry stuff here if wanted

;-----------------------------------
; Interface configuration


!define MUI_ICON "${INST_FOLDER}\davinci.ico"
!define MUI_UNICON "${INST_FOLDER}\davinci.ico"

; Recommended to be 150 x 57
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP "${INST_FOLDER}\davinci.bmp"
!define MUI_HEADERIMAGE_UNBITMAP "${INST_FOLDER}\davinci.bmp"

!define MUI_ABORTWARNING

; Recommended to be 164 x 314
!define MUI_WELCOMEFINISHPAGE_BITMAP "${INST_FOLDER}\davinci2.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "${INST_FOLDER}\davinci2.bmp"

!define MUI_WELCOMEPAGE_TITLE "Welcome to the Davinci-${VERSION} Setup"


!define MUI_WELCOMEPAGE_TEXT "http://davinci.asu.edu$\r$\n$\r$\nThis will install Davinci-${VERSION} on your computer.$\r$\n$\r$\nThis will also install Gnuplot 4.2, Imgv 3.1.5 and Ghostscript 8.57 internally by default.$\r$\n$\r$\nDavinci is an interpreted language that looks and feels a lot like C, but has additional vector oriented features that make working with blocks of data a lot easier. This makes davinci well suited for use as a data processing tool, allowing symbolic and mathematical manipulation of hyperspectral data for imaging spectroscopy applications."


!define MUI_FINISHPAGE_LINK "Davinci Website"
!define MUI_FINISHPAGE_LINK_LOCATION "http://davinci.asu.edu"

!define MUI_FINISHPAGE_RUN "$INSTDIR\davinci.bat"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\README.txt"




;-----------------------------------
; Pages

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${INST_FOLDER}\LICENSE.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH


!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH


;-----------------------------------
; Languages

; Extremely unlikely we'll ever add support for other languages
!insertmacro MUI_LANGUAGE "English"


;-----------------------------------
; Installer Sections


#default section start
Section "-Core"
	SetOutPath $INSTDIR
	File ${INST_FOLDER}\*

	SetOutPath $PROFILE
	File ${INST_FOLDER}\.dvrc

	WriteUninstaller $INSTDIR\uninstaller.exe
SectionEnd

SectionGroup /e "Davinci" davinci_id

	Section "Scripts Library" library_id
		SetOutPath $INSTDIR
		File /r ${INST_FOLDER}\library
	SectionEnd

	Section "Modules" modules_id
		SetOutPath $INSTDIR
		File /r ${INST_FOLDER}\modules
	SectionEnd

	Section "Examples" examples_id
		SetOutPath $INSTDIR
		File /r ${INST_FOLDER}\examples
	SectionEnd

	Section "Unix Utilities" unix_id
		SetOutPath $INSTDIR
		File /r ${INST_FOLDER}\bin
	SectionEnd

	Section "imgv" imgv_id
		SetOutPath $INSTDIR
		File /r ${INST_FOLDER}\imgv
	SectionEnd



	Section "Shortcuts" shortcuts_id
		CreateShortcut $DESKTOP\Davinci-${VERSION}.lnk $INSTDIR\davinci.bat "" $INSTDIR\davinci.ico

		CreateDirectory $STARTMENU\Davinci-${VERSION}
		CreateShortcut $STARTMENU\Davinci-${VERSION}\Davinci.lnk $INSTDIR\davinci.bat "" $INSTDIR\davinci.ico
		CreateShortcut "$STARTMENU\Davinci-${VERSION}\Davinci Online Reference.lnk" http://davinci.asu.edu/ "" $INSTDIR\davinci.ico
		CreateShortcut $STARTMENU\Davinci-${VERSION}\Uninstall.lnk $INSTDIR\uninstaller.exe
	SectionEnd

SectionGroupEnd

Section "Gnuplot" gnuplot_id
	SetOutPath $INSTDIR
	File /r ${INST_FOLDER}\Gnuplot
SectionEnd

Section /o "ImageJ" imagej_id
	SetOutPath $INSTDIR
	File /r ${INST_FOLDER}\ImageJ
SectionEnd

Section "Ghostscript" ghostscript_id
	SetOutPath $INSTDIR
	File /r ${INST_FOLDER}\Ghostscript
SectionEnd



;-----------------------------------
; Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${davinci_id} "Core Davinci components"
	!insertmacro MUI_DESCRIPTION_TEXT ${library_id} "A collection of Davinci scripts that contain user defined functions."
	!insertmacro MUI_DESCRIPTION_TEXT ${modules_id} "Davinci loadable modules (e.g. $\"thm$\")."
	!insertmacro MUI_DESCRIPTION_TEXT ${examples_id} "Example data files."
	!insertmacro MUI_DESCRIPTION_TEXT ${unix_id} "Standard Unix utilities (cp, mv, cat etc.) used in some Davinci scripts."
	!insertmacro MUI_DESCRIPTION_TEXT ${imgv_id} "Imgv (Image Viewer) is a cross-platform, open source, image viewer."
	!insertmacro MUI_DESCRIPTION_TEXT ${shortcuts_id} "Installs shortcuts for the Desktop and Start Menu."


	!insertmacro MUI_DESCRIPTION_TEXT ${gnuplot_id} "Gnuplot is a versatile program that can generate plots of functions and data."
	!insertmacro MUI_DESCRIPTION_TEXT ${imagej_id} "ImageJ is a java-based image processing tool (requires Java Virtual Machine)."
	!insertmacro MUI_DESCRIPTION_TEXT ${ghostscript_id} "Ghostscript is a suite for writing PDF files. It may be needed for some scripts."


!insertmacro MUI_FUNCTION_DESCRIPTION_END


;------------------------------------
; Installer Functions

;Function .onInit
;
;FunctionEnd





;------------------------------------
; Uninstaller section

; NOTE in Uninstall section $INSTDIR contains location of uninstaller
; not necessarily the same value as in the installer section
Section "Uninstall"
	RMDir /r /REBOOTOK $INSTDIR

	; should we delete this?
	Delete $PROFILE\.dvrc

	Delete $DESKTOP\Davinci-${VERSION}.lnk
	RMDir /r $STARTMENU\Davinci-${VERSION}


SectionEnd


