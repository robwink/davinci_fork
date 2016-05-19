JAVA=/java
TMP=.swig-tmp
swig -java -package edu.asu.msff.iomedley -module iomedley iomedley.h
echo '#include "iomedley.h"' > iomedley_wrap_fixed.c
cat iomedley_wrap.c >> iomedley_wrap_fixed.c
gcc -I$JAVA/include -I$JAVA/include/win32 -I. -Wl,--add-stdcall-alias -shared -o iomedley.dll iomedley_wrap_fixed.c libiomedley.a -lz
mkdir -p $TMP/edu/asu/msff/iomedley
mv *.java $TMP/edu/asu/msff/iomedley
pushd $TMP
jar cvf iomedley.jar edu
popd
mv $TMP/iomedley.jar .
rm -rf edu iomedley_wrap.c iomedley_wrap_fixed.c
