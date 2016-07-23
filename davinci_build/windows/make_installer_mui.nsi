!include LogicLib.nsh

!include "MUI2.nsh"


!define VERSION "2.17"
!define INST_FOLDER "davinci_win"


Name "Davinci-${VERSION}"

#installer name
OutFile "Davinci-${VERSION}-Setup.exe"

# change for 32/64 bit?  We should just stop releasing 32 bit
InstallDir "$PROGRAMfILES64\Davinci-${VERSION}"

#Do registry stuff here if wanted



!insertmacro MUI_PAGE_LICENSE ${INST_FOLDER}\LICENSE.txt
#!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES


; set language
!insertmacro MUI_LANGUAGE "English"


#default section start
Section
	SetOutPath $INSTDIR
	File /r ${INST_FOLDER}\*

	CreateShortcut $DESKTOP\Davinci-${VERSION}.lnk $INSTDIR\davinci.bat "" $INSTDIR\davinci.ico

	CreateDirectory $STARTMENU\Davinci-${VERSION}
	CreateShortcut $STARTMENU\Davinci-${VERSION}\Davinci.lnk $INSTDIR\davinci.bat "" $INSTDIR\davinci.ico
	CreateShortcut "$STARTMENU\Davinci-${VERSION}\Davinci Online Reference.lnk" http://davinci.asu.edu/ "" $INSTDIR\davinci.ico
	CreateShortcut $STARTMENU\Davinci-${VERSION}\Uninstall.lnk $INSTDIR\uninstaller.exe


	WriteUninstaller $INSTDIR\uninstaller.exe

SectionEnd


#NOTE in Uninstall section $INSTDIR contains location of uninstaller
#not necessarily the same value as in the installer section
Section "Uninstall"
	#Delete $INSTDIR\*
	RMDir /r /REBOOTOK $INSTDIR
	Delete $DESKTOP\Davinci-${VERSION}.lnk
	RMDir /r $STARTMENU\Davinci-${VERSION}

	#Delete $STARTMENU\Davinci-${VERSION}\Davinci.lnk

SectionEnd



UninstPage uninstConfirm
UninstPage instfiles
