#!/bin/sh 

#MINGW workaround. 
#This needs to eventually get integrated in Makefile.am.
#This creates davinci.dll, davinci.exe and modules.
#Run:
#
# ./makwin.sh [davinci_source]
#



davinci_src="$1"

current_dir=`pwd`



if [ "$davinci_src" = "" ]; then 
    davinci_src="../"
fi


cd $davinci_src

LIBS=""
function filllibs() {
	name="$1"
	len=`expr ${#name} + 3`
	libs=`cat Makefile | grep -E ^$name  | cut -c$len-`
	LIBS="$LIBS $libs"
}
	
filllibs MY_IOMEDLEY_LDADD
filllibs MY_MODULES_LIB
#filllibs MY_READLINE_LDADD
filllibs MINGW32_LIBREGEX_LDADD
filllibs LIBS
#filllibs MINGW32_LIBWSOCK32_LDADD
LIBS="$LIBS libltdl/.libs/libltdlc.a"





echo "Create davinci.dll"
objs=`ls *.o | grep -v main.o` 
davinci_dll="gcc -g -O2 -shared -o davinci.dll $objs  -Wl,--export-all-symbols  $LIBS"
echo $davinci_dll
echo `$davinci_dll`

#Create davinci.exe
`rm -f libdavinci.la`
davinci_exe="gcc -g -O2 -Iiomedley -I. -o davinci.exe main.c -L. -ldavinci -lreadline"
echo $davinci_exe
echo `$davinci_exe`

set -v
#Create modules
cd modules/thm
gcc -shared -o thm.lai mod_thm.o -L../../ -ldavinci 
cd ../../

cd modules/cse
gcc -shared -o cse.lai mod_cse.o -L../../ -ldavinci
cd ../../

cd modules/kjn
gcc -shared -o kjn.lai mod_kjn.o -L../../ -ldavinci
cd ../../

cd modules/pnm
gcc -shared -o pnm.lai pnm_mod.o -L../../ -ldavinci
cd ../../

mkdir -p  $current_dir/modules

mv davinci.dll $current_dir
mv davinci.exe $current_dir
mv modules/thm/thm.lai $current_dir/modules
mv modules/cse/cse.lai $current_dir/modules
mv modules/kjn/kjn.lai $current_dir/modules
cd $current_dir


