#!/bin/bash 

#excecute this script from the davinci/build_utils/mac svn directory 
#include the standard davinci_build utils directory
# 
# check out copies of davinci, readline, davinci_build (as build_utils) from svn
#
# Script:  compile.sh  


#check for old versions of installed davinci...if they exist we link on them---bug in davinci.
#set -x
if [ -e /usr/local/davinci ] 
then
	echo Found a previous copy of davinci in /usr/local/davinci
	echo Please remove this installation of davinci before building
	exit
fi

#set up what things to build
release=1
packages=1

#force building fo libraries and davinci
cfitsio=0
openmotif=0
hdf=0
x11var=0
aquavar=0

#force build all
force=0

#set up environment variables
dir=$PWD
pmproj=$dir/pmproj
dep=$dir/dependencies
dep_install=$dir/install/dep/usr/local/davinci
x11=$dir/install/x11/usr/local/davinci
aqua=$dir/install/aqua/usr/local/davinci

#get the version of davinci from version.h to attach to filename
version=`grep 'daVinci Version #' $dir/../../version.h | sed -e 's/\(.*\)#\(.*\)";/\2/'`
if [ "${version}" = "" ] 
then
  echo "Davinci version was not found"
  exit
fi

#
echo Finding and Removing \"svn-commit.tmp*\"
find . -name "svn-commit.*tmp*" | xargs rm


#make installation directories
mkdir -p $dep_install $x11 $aqua

#make dependencies for davinci
if [ ! -e $dep_install/lib/libcfitsio.a ] || [ "$force" -eq 1 ] || [[ "$cfitsio" -eq 1 ]]
then 
	echo Building cfitsio
	cd $dep/cfitsio
	./configure --prefix=$dep_install
	make
	make install
	mkdir -p $dep_install/include/cfitsio/
	cd $dep_install/include/cfitsio/
	ln -s ../fitsio*.h .
else 
	echo Skipping cfitsio because it is built
fi

if [ ! -e $dep_install/lib/libXm.2.0.1.dylib ] || [ "$force" -eq 1 ] || [[ "$openmotif" -eq 1 ]]
then
	echo Building lesstif
#	cd $dep/openmotif-2.3.0
	cd $dep/lesstif-0.95.0
	./configure --enable-static=no --prefix=/usr/local/davinci
	make
	make install DESTDIR=$dir/install/dep
else
	echo Skipping lesstif because it is built
fi

if [ ! -e $dep_install/lib/libhdf5.0.0.0.dylib ] || [ "$force" -eq 1 ]  || [[ "$hdf" -eq 1 ]]
then
	echo Building hdf5
	cd $dep/hdf5-1.6.5
	./configure --enable-hdf5v1_4 --enable-static=no --prefix=/usr/local/davinci
	make
	cp $dep/hdf5-1.6.5/src/.libs/libhdf5.0.0.0.dylib $dep_install/lib
	cp $dep/hdf5-1.6.5/src/.libs/libhdf5.lai $dep_install/lib/libhdf5.la
	cp $dep/hdf5-1.6.5/src/hdf5.h $dep_install/include
	cp $dep/hdf5-1.6.5/src/libhdf5.settings $dep_install/lib
	cp $dep/hdf5-1.6.5/hl/src/.libs/libhdf5_hl.0.0.0.dylib $dep_install/lib
	cp $dep/hdf5-1.6.5/hl/src/.libs/libhdf5_hl.lai $dep_install/lib/libhdf5_hl.la

	cd $dep_install/lib
	ln -s libhdf5.0.0.0.dylib libhdf5.0.dylib
	ln -s libhdf5.0.0.0.dylib libhdf5.dylib
  ln -s libhdf5_hl.0.0.0.dylib libhdf5_hl.0.dylib
	ln -s libhdf5_hl.0.0.0.dylib libhdf5_hl.dylib
else	
	echo Skipping hdf5 because it is built
fi

rm -rf $dep_install/bin $dep_install/man $dep_install/share $dep_install/doc $dep_install/LessTif $dir/install/dep/sw

#start by building davinci w/ x11
cd $dir/../../

#start by building iomedley
cd iomedley
./configure
make
cd ..

if [ ! -e $x11/bin/davinci ] || [ "$force" -eq 1 ] || [[ "$x11var" -eq 1 ]]
then
	echo Building davinci for X11
	./configure --prefix=/usr/local/davinci --with-hdf5=$dep_install --with-motif=$dep_install --with-cfitsio=$dep_install --without-library --without-examples --enable-aqua	
	make
	rm -rf  $x11/lib/* $x11/share/* $x11/include/* $x11/bin/*
	make install DESTDIR=$dir/install/x11
	rm -rf $x11/man $x11/info
else
	echo Skipping davinci for X11 because it is built
fi

#next build davinci w/o x11
if [ ! -e $aqua/bin/davinci ] || [ "$force" -eq 1 ] || [[ "$aquavar" -eq 1 ]]
then
	echo Building davinci for aqua
	./configure --prefix=/usr/local/davinci --with-hdf5=$dep_install --with-cfitsio=$dep_install --without-library --without-examples --enable-aqua --without-x
	make
	rm -rf  $aqua/lib/* $aqua/share/* $aqua/include/* $aqua/bin/*
	make install DESTDIR=$dir/install/aqua
	rm -rf $aqua/man $aqua/info
else
	echo Skipping davinci for aqua because it is built
fi

if [[ "$packages" -eq 1 ]] 
then
	echo Building Packages
	#build the packages and copy library and exampes for packaging
	cd $dir
	
	echo Copying library and examples from the davinci source directory
	components="Davinci_for_Mac_Installer/Components"
	find components/library/davinci/examples -name ".svn" | xargs rm -rf
	find components/library/davinci/library -name ".svn" | xargs rm -rf
	cp -r $dir/../../examples components/library/davinci
	cp -r $dir/../../library components/library/davinci
 
	#core davinci stuff
	echo Making davinci_app_aqua.pkg
	rm -rf $components/davinci_app_aqua.pkg
	/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker -build -proj $pmproj/davinci_app_aqua.pmproj -p $components/davinci_app_aqua.pkg

	echo Making davinci_app_x11.pkg
	rm -rf $components/davinci_app_x11.pkg
	/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker -build -proj $pmproj/davinci_app_x11.pmproj -p $components/davinci_app_x11.pkg

	echo Making davinci_aqua.pkg
	rm -rf $components/davinci_aqua.pkg
	/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker -build -proj $pmproj/davinci_aqua.pmproj -p $components/davinci_aqua.pkg

	echo Making davinci_x11.pkg
	rm -rf $components/davinci_x11.pkg
	/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker -build -proj $pmproj/davinci_x11.pmproj -p $components/davinci_x11.pkg

	echo Making davinci_dependencies.pkg
	rm -rf $components/davinci_dependencies.pkg
	/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker -build -proj $pmproj/davinci_dependencies.pmproj -p $components/davinci_dependencies.pkg

	echo Making davinci_dependencies2.pkg
	rm -rf $components/davinci_dependencies2.pkg
	/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker -build -proj $pmproj/davinci_dependencies2.pmproj -p $components/davinci_dependencies2.pkg

	#dvrc and library stuff
	echo Making davinci_dvrc_aqua.pkg
	rm -rf $components/davinci_dvrc_aqua.pkg
	/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker -build -proj $pmproj/davinci_dvrc_aqua.pmproj -p $components/davinci_dvrc_aqua.pkg

	echo Making davinci_dvrc_x11.pkg
	rm -rf $components/davinci_dvrc_x11.pkg
 	/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker -build -proj $pmproj/davinci_dvrc_x11.pmproj -p $components/davinci_dvrc_x11.pkg

	echo Making davinci_library.pkg
	rm -rf $components/davinci_library.pkg
	/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker -build -proj $pmproj/davinci_library.pmproj -p $components/davinci_library.pkg

	#assemble the meta package
	echo Assemble meta package
	rm -rf Davinci_for_Mac_Installer/Davinci_for_Mac.mpkg
	/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker -build -proj $pmproj/Davinci_for_Mac_meta.pmproj -p Davinci_for_Mac_Installer/Davinci_for_Mac.mpkg

	#make the disk image
	echo Building Disk image as davinci-${version}-${release}.dmg
	if [ -e  $dir/davinci-${version}-${release}.dmg ]
	then
		rm $dir/davinci-${version}-${release}.dmg
	fi
	hdiutil create -srcdir $dir/Davinci_for_Mac_Installer $dir/davinci-${version}-${release}.dmg
fi
