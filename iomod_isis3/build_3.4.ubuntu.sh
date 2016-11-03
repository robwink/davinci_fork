#! /bin/sh
echo Compiling ISIS3 module.... 3.4 version
ISIS3DIR=/localdata/isis3/isis
QTSRCDIR=/usr/include
ARCH=64

LD_RUN_3PTY=$ISIS3DIR/3rdParty/lib
LD_RUN_ISIS=$ISIS3DIR/lib

echo $LD_RUN_3PTY $LD_RUN_ISIS

g++ -DHAVE_CONFIG_H -m$ARCH -fPIC -g -I.. -I../libltdl -I$ISIS3DIR/inc -I$QTSRCDIR -I/$QTSRCDIR/qt4/ -I/$QTSRCDIR/qt4/QtCore -Wall iomod_isis3.4.cpp -c -o iomod_isis34.o
if [ $? != 0 ]; then
    echo Compile failed.
    exit 1
fi
echo Linking ISIS3 module...
g++ -m$ARCH -fPIC -shared -Wl,-soname,isis34.dvio -Wl,-rpath=$LD_RUN_3PTY -Wl,-rpath=$LD_RUN_ISIS -o isis34.dvio iomod_isis34.o  -L$LD_RUN_ISIS -L$LD_RUN_3PTY -lisis3 -lkdu_a63R -lgsl -lgeos_c -lqwt -lxerces-c -lprotobuf -lQtDBus
if [ $? != 0 ]; then
    echo Link failed.
    exit 1
fi
echo "Build of ISIS3 I/O module successful."
