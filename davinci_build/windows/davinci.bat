@echo off

set HOME=%USERPROFILE%
set DV_HOME=%~dp0
set SYSPATH=%PATH%

set PATH=%DV_HOME%
set PATH=%PATH%;%DV_HOME%bin
set PATH=%PATH%;%DV_HOME%Gnuplot\bin
set PATH=%PATH%;%DV_HOME%Ghostscript\bin
set PATH=%PATH%;%DV_HOME%Ghostscript\lib
set PATH=%PATH%;%DV_HOME%ImageJ
set PATH=%PATH%;%DV_HOME%imgv

REM This was my simple image viewer
REM set PATH=%PATH%;%DV_HOME%dv_img


if "%TMPDIR%" == "" set TMPDIR=%TEMP%

REM change this later if I ever add pixel values
REM even though imgv doesn't have them either
REM set DEFAULT_DV_VIEWER=dv_img.exe

set DEFAULT_DV_VIEWER=imgv.exe

set IMGV_HOME=%DV_HOME%\imgv

set ALTERNATE_DV_VIEWER=ImageJ.exe

if "%DV_MOD_PATH%" == "" set DV_MOD_PATH=%DV_HOME%modules
if "%DV_LIB%" == "" set DV_LIB=%DV_HOME%library
if "%DVHELP%" == "" set DVHELP=%DV_HOME%dv.gih
if "%DV_EX%" == "" set DV_EX=%DV_HOME%examples
if "%GPLOT_CMD%" == "" set GPLOT_CMD=pgnuplot.exe
if "%EDITOR%" == "" set EDITOR=notepad

if "%USER%" == "" set USER=%USERNAME%
if "%HOST%" == "" set HOST=%COMPUTERNAME%
if "%GNUPLOT_PS_DIR%" == "" set GNUPLOT_PS_DIR=%DV_HOME%Gnuplot\bin\share\PostScript

rem Ghostscript
set GS_LIB=%DV_HOME%\Ghostscript\fonts;%DV_HOME%\Ghostscript\lib


rem FIX THE IMAGE VIEWER

if "%DV_VIEWER%" == "" goto  setviewer
goto endviewer
:setviewer
if exist C:\WINDOWS\system32\javaw.exe goto javafound
echo "Java not found"
goto defaultviewer

:javafound
if exist "%DV_HOME%ImageJ/%ALTERNATE_DV_VIEWER%" goto imagejfound
goto defaultviewer

:imagejfound
set DV_VIEWER=%ALTERNATE_DV_VIEWER%
goto endviewer

:defaultviewer
set DV_VIEWER=%DEFAULT_DV_VIEWER%
:endviewer

rem Restore Path (by appending at the end)
set PATH=%PATH%;%SYSPATH%; 
echo Environment:
echo PATH=%PATH%
echo DV_HOME=%DV_HOME%
echo TMPDIR=%TMPDIR%
echo DV_VIEWER=%DV_VIEWER%
echo DV_MOD_PATH=%DV_MOD_PATH%
echo DV_LIB=%DV_LIB%
echo DVHELP=%DVHELP%
echo DV_EX=%DV_EX%
echo GPLOT_CMD=%GPLOT_CMD%
echo EDITOR=%EDITOR%

echo. 

cd %HOME%
"%DV_HOME%"davinci.exe -l "%HOME%/.dvlog" %1 %2 %3 %4 %5 %6 %7 %8 %9

