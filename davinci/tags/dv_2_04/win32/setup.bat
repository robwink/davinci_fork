echo This file will setup and compile your davinci package
echo You MUST have a copy of Visual C++ 5.0 or higher and your PATH
echo variable must contain the path to the VC++ binaries

echo off

call vcvars32.bat

copy *.c ..\.
copy readline.dll c:\windows\system\.
copy values.h ..\.
copy config.h ..\.

copy davinci.mak ..\.

cd ..
nmake /f davinci.mak CFG="davinci - Win32 Release"

move release\davinci.exe .

echo Done!
