#!/bin/bash 

#excecute this script from the davinci/build_utils/mac svn directory 
#include the standard davinci_build utils directory
# 
# check out copies of davinci, readline, davinci_build (as build_utils) from svn
#
# Script:  build_mac.sh  

#check for old versions of installed davinci...if they exist we link on them---bug in davinci.
#set -x
PATH=/bin:/usr/bin:/usr/X11R6/bin:/opt/local/bin:/usr/local/bin/:/sw/bin

PKGMKR=/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker
if [[ ! -e "$PKGMKR" ]]; then
	PKGMKR=/Applications/PackageMaker.app/Contents/MacOS/PackageMaker
fi

#set up what things to build
release=1
packages="davinci dvlib"

tgt_os=10.8
tgt_arch=x86_64
#tgt_arch=i386

[[ -n "$1" ]] && tgt_os="$1"
[[ -n "$2" ]] && tgt_arch="$2"


#force building fo libraries and davinci
cfitsio=0
hdf=0
readline=0
gnuplot=0
libxml2=0

#force build all
force=1

#set up environment variables
dir=$PWD
source=$dir/../../

# "dep" contains external package dependencies that need to be built and installed
# in order for davinci to work. These include "hdf5", ...
dep=$dir/dependencies

# "comp" contains package components including a package template, postinstall
# scripts, the davinci library,  licenses, inputrc ...
comp=$dir/components/

# "app_dir" is location on Mac OS X where applications are installed
app_dir=/Applications

# "dv_app" is the name of davinci application
dv_app=davinci.app
dv_app_dir=${app_dir}/${dv_app}

# Location where dependencies and various pieces from comp will end up at
# CAUTION: gnuplot & iomedley use different versions of libraries, e.g. libpng;
# installing their dependencies in the same dir will cause issues, hence gnuplot
# and its dependencies have been moved to their own sub-directory.
install_dir=${dv_app_dir}/Contents/Resources/
gplot_install_dir=${dv_app_dir}/Contents/Resources/gnuplot


# Generates the 99compontent-contents.xml file for package building
function gen_contents_xml () {
	depth=$1
	ftype=$2
	fpath=$3

	indent=$((depth*2))
	fname=`basename $fpath`
	if [ $ftype == "d" ]; then
		if [ $depth -eq 0 ]; then
			printf '<?xml version="1.0"?>\n'
			printf '<pkg-contents spec="1.12">\n'
			printf '%*s<f n="%s" o="root" g="admin" p="16892" pt="%s" m="true" t="file">\n' $indent "" "$fname" "$fpath"
		else
			printf '%*s<f n="%s" o="root" g="admin" p="16893">\n' $indent "" "$fname"
		fi

		find $fpath -maxdepth 1 -ls | tail +2 | awk '{ print substr($3,1,1),$11 }' | while read ftype fpath
		do
			fname=`basename $fpath`
			if [ $ftype == "d" ]; then
				gen_contents_xml $((depth+1)) $ftype $fpath 
				if [ $? -ne 0 ]; then
					return 1
				fi
			else
				printf '%*s<f n="%s" o="root" g="admin" p="33277"><mod>mode</mod></f>\n'  $((indent+2)) "" "$fname"
			fi
		done
		printf '%*s<mod>mode</mod>\n' $((indent+2)) ""
		printf '%*s</f>\n' $indent ""
		if [ $depth -eq 0 ]; then
			printf '</pkg-contents>\n'
		fi
	else
		printf '%*s<f n="%s" o="root" g="admin" p="33277"><mod>mode</mod></f>\n'  $indent "" "$fname"
	fi
}

function failed() {
	echo $1
	exit 1
}


if [ "`echo $packages | grep davinci`" ]; then
	echo "Creating ${dv_app} application dir ${dv_app_dir}"
	mkdir ${dv_app_dir}
	[[ $? -eq 0 ]] || failed "Unable to create ${dv_app_dir}."

	echo "Creating dir ${install_dir}/lib"
	mkdir -p ${install_dir}/lib
	[[ $? -eq 0 ]] || failed "Unable to create ${install_dir}/lib."

	echo "Copying ${dv_app}"
	tar -C $comp --exclude '\.svn' --exclude CVS --exclude '\.DS' --exclude '\.cvs' -cf - ${dv_app} | tar -C ${app_dir} -xf - 
	[[ $? -eq 0 ]] || failed "Unable to copy ${dv_app}."

	echo "Copying licenses"
	cat ${comp}/Licenses/davinci_license.txt ${comp}/Licenses/{cfitsio_license,curl_license,hdf5_license}.txt >  ${install_dir}/license.txt

	#echo "Copying dvrc"
	#cp -r ${comp}/dvrc ${install_dir}
	#[[ $? -eq 0 ]] || failed "Unable to copy dvrc."

	#echo "Copying postinstall scripts for dvrc"
	#tar -C $comp --exclude '\.svn' --exclude CVS --exclude '\.DS' --exclude '\.cvs' -cf - extras-dvrc | tar -C ${dmg} -xf - 
	#[[ $? -eq 0 ]] || failed "Unable to copy postinstall scripts for dvrc."

	#echo "Copying inputrc"
	#cp -r ${comp}/inputrc ${install_dir}
	#[[ $? -eq 0 ]] || failed "Unable to copy inputrc."

	#echo "Copying postinstall scripts for inputrc"
	#tar -C $comp --exclude '\.svn' --exclude CVS --exclude '\.DS' --exclude '\.cvs' -cf - extras-inputrc | tar -C ${dmg} -xf - 
	#[[ $? -eq 0 ]] || failed "Unable to copy postinstall scripts for inputrc."

	echo "Copying misc"
	tar -C $comp/misc --exclude '\.svn' --exclude CVS --exclude '\.DS' --exclude '\.cvs' -cf - . | tar -C ${install_dir}/lib -xf - 
	[[ $? -eq 0 ]] || failed "Unable to copy misc."

	echo "Getting davinci version number"
	#get the version of davinci from version.h to attach to filename
	version=`grep 'davinci ' $source/version.h | sed -e 's/\(.*\) \(.*\)";/\2/'`
	[[ "${version}" != "" ]] || failed "Davinci version was not found. Unable to build package."
	major=`echo $version | cut -d. -f1`
	minor=`echo $version | cut -d. -f2`
	echo "davinci version number found as major:$major minor:$minor"

	#echo Finding and Removing \"svn-commit.tmp*\"
	#find . -name "svn-commit.*tmp*" | xargs rm -f

	# make dependencies for davinci
	export CC="gcc -arch $tgt_arch" CXX="g++ -arch $tgt_arch"

	#export CC="gcc-4.0 -m32" CXX="g++ -m32"
	#export CC="gcc-4.0 -arch i386 -isysroot /Developer/SDKs/MacOSX10.4u.sdk -mmacosx-version-min=10.4"
	#export CXX="g++-4.0 -arch i386 -isysroot /Developer/SDKs/MacOSX10.4u.sdk -mmacosx-version-min=10.4"
	#export CC="gcc -arch ${tgt_arch} -isysroot /Developer/SDKs/MacOSX${tgt_os_min}.sdk -mmacosx-version-min=${tgt_os_min}"
	#export CXX="g++ -arch ${tgt_arch} -isysroot /Developer/SDKs/MacOSX${tgt_os_min}.sdk -mmacosx-version-min=${tgt_os_min}"

	# build cfitsio
	cd $source
	if [ ! -e ${install_dir}/lib/libcfitsio.a ] || [ "$force" -eq 1 ] || [[ "$cfitsio" -eq 1 ]]
	then 
		echo "Building cfitsio"
		cd $dep/cfitsio
		./configure --prefix=$install_dir 
		[[ $? -eq 0 ]] || failed "Unable to configure cfitsio."
		make clean
		make install
		[[ $? -eq 0 ]] || failed "Unable to build/install cfitsio."

		mkdir ${install_dir}/include
		mkdir ${install_dir}/include/cfitsio
		cd ${install_dir}/include/cfitsio/ \
		&& ln -s ../fitsio*.h .
		[[ $? -eq 0 ]] || failed "Link creation failed for cfitsio."
	else 
		echo Skipping cfitsio because it is built
	fi

	#build hdf5
	cd $source
	if [ ! -e ${install_dir}/lib/libhdf5.dylib ] || [ "$force" -eq 1 ]  || [[ "$hdf" -eq 1 ]]
	then
		echo "Building hdf5"
		cd $dep/hdf5-1.8.4
		./configure --enable-hdf5v1_4 --enable-static=no --prefix=$install_dir 
		[[ $? -eq 0 ]] || failed "Unable to configure hdf5."
		make clean
		make install
		[[ $? -eq 0 ]] || failed "Unable to build/install hdf5."
		cd ..
	else	
		echo Skipping hdf5 because it is built
	fi

	# build libxml2
	cd $source
	if [ ! -e ${install_dir}/lib/libxml2.a ] || [ "$force" -eq 1 ] || [[ "$libxml2" -eq 1 ]]
	then 
		echo "Building libxml2"
		cd $dep/libxml2-2.9.1
		./configure --prefix=$install_dir 
		[[ $? -eq 0 ]] || failed "Unable to configure libxml2."
		make clean
		make install
		[[ $? -eq 0 ]] || failed "Unable to build/install libxml2."
	else 
		echo Skipping libxml2 because it is built
	fi

	#build readline
	cd $source
	if [ ! -e ${install_dir}/lib/libreadline.a ] || [ "$force" -eq 1 ]  || [[ "$readline" -eq 1 ]]
	then
		echo "Building readline"
		cd $dep/readline-5.2
		./configure --enable-shared=no --prefix=$install_dir 
		[[ $? -eq 0 ]] || failed "Unable to configure readline."
		make clean
		make install
		[[ $? -eq 0 ]] || failed "Unable to build/install readline."
		cd ..
	else	
		echo Skipping readline because it is built
	fi


	#
	# gnuplot dependencies
	#

	# build libpng - a gd dependency
	cd $source
	if [ ! -e ${gplot_install_dir}/lib/libpng.dylib ] || [ "$force" -eq 1 ]  || [[ "$gnuplot" -eq 1 ]]
	then
		echo "Building libpng"
		cd $dep/libpng-1.5.1/
		./configure --prefix=$gplot_install_dir 
		[[ $? -eq 0 ]] || failed "Unable to configure libpng."
		make clean && make install
		[[ $? -eq 0 ]] || failed "Unable to build/install libpng."
		cd ..
	else
		echo Skipping libpng because it is built
	fi

	# build gd - a gnuplot dependency 
	cd $source
	if [ ! -e ${gplot_install_dir}/lib/libgd.dylib ] || [ "$force" -eq 1 ]  || [[ "$gnuplot" -eq 1 ]]
	then
		echo "Building gd"
		cd $dep/gd-2.0.35/
		./configure --with-x --x-includes=/usr/X11/include --x-libraries=/usr/X11/lib --prefix=$gplot_install_dir --with-png=$gplot_install_dir
		[[ $? -eq 0 ]] || failed "Unable to configure gd."
		make clean && make install
		[[ $? -eq 0 ]] || failed "Unable to build/install gd."
		cd ..
	else
		echo Skipping gd because it is built
	fi

	# build libpdf - a gnuplot dependency
	cd $source
	if [ ! -e ${gplot_install_dir}/lib/libpdf.dylib ] || [ "$force" -eq 1 ]  || [[ "$gnuplot" -eq 1 ]]
	then
		echo "Building libpdf"
		cd $dep/PDFlib-Lite-7.0.5/
		./configure --without-java --without-tcl --without-ruby --without-java --without-perl --without-py --disable-dynamic --prefix=$gplot_install_dir
		[[ $? -eq 0 ]] || failed "Unable to configure libpdf."
		make clean && make && make install
		[[ $? -eq 0 ]] || failed "Unable to build/install libpdf."
		cd ..
	else
		echo Skipping libpdf because it is built
	fi


	# build gnuplot 
	cd $source
	if [ ! -e ${gplot_install_dir}/bin/gnuplot ] || [ "$force" -eq 1 ]  || [[ "$gnuplot" -eq 1 ]]
	then
		echo "Building gnuplot"
		export PATH=${install_dir}/bin:$PATH
		cd $dep/gnuplot-4.4.2/
		# TODO Building without readline causes the mouse readout in the gnuplot to not work any more.
		./configure --without-tutorial --with-readline=builtin --without-lisp-files --without-latex --x-includes=/usr/X11/include --x-libraries=/usr/X11/lib --with-x --disable-wxwidgets --without-cairo --enable-backwards-compatibility --with-gd=${gplot_install_dir} --with-pdf=${gplot_install_dir} --prefix=${gplot_install_dir}
		[[ $? -eq 0 ]] || failed "Unable to configure gnuplot."
		make clean && make && make install
		[[ $? -eq 0 ]] || failed "Unable to build/install gnuplot."
		cd ..
	else
		echo Skipping gnuplot because it is built
	fi

	#start by building davinci w/ x11
	cd $source

	#start by building iomedley
	echo "Building iomedley"
	cd iomedley
	./configure --prefix=${install_dir}
	[[ $? -eq 0 ]] || failed "Failed to configure iomedley."
	make clean && make 
	[[ $? -eq 0 ]] || failed "Failed to build iomedley."


	cd $source

	if [ ! -e $install_dir/bin/davinci ] || [ "$force" -eq 1 ] || [[ "$x11var" -eq 1 ]]
	then
		echo "Building davinci"
		./configure --prefix=$install_dir --with-hdf5=${install_dir} --with-cfitsio=${install_dir} --with-gnuplot=${install_dir}/bin/gnuplot --with-viewer=${install_dir}/ImageJ/display.sh --without-library --without-examples --with-pic  --with-libxml2=${install_dir} --with-libxml2_hdr=${install_dir}/include/libxml2
		make clean
		make install
		[[ $? -eq 0 ]] || failed "Unable to build/install davinci."
		#rm -rf $install_dir/man $install_dir/info
	else
		echo Skipping davinci for X11 because it is built
	fi

	#copy the library and exmaples
	#echo "Copying davinci library"
	#cp -r $source/library $install_dir
	#[[ $? -eq 0 ]] || failed "Unable to copy davinci library."
	#find $install_dir/library -name '.svn' -prune -type d -exec rm -rf {} \;

	#echo "Copying davinci examples"
	#cp -r $source/examples $install_dir
	#[[ $? -eq 0 ]] || failed "Unable to copy davinci examples."
	#find $install_dir/examples -name '.svn' -prune -type d -exec rm -rf {} \;

	echo 
	echo

	#build the packages and copy library and exampes for packaging
	echo '**** Building davinci'

	cd $dir
	echo "Generating davinci contents xml"
	gen_contents_xml 0 "d" $dv_app_dir > davinci.pmdoc/01davinci-contents.xml
	[[ $? -eq 0 ]] || failed "Generating davinci contents xml failed."

	echo "Generating xdefaults contents xml"
	gen_contents_xml 0 "d" $dir/components/xdefaults > davinci.pmdoc/05xdefaults-contents.xml
	[[ $? -eq 0 ]] || failed "Generating xdefaults contents xml failed."
	
	echo "Generating inputrc contents xml"
	#gen_contents_xml 0 "d" $dv_app_dir/Contents/Resources/inputrc > davinci.pmdoc/04inputrc-contents.xml
	gen_contents_xml 0 "d" $dir/components/inputrc > davinci.pmdoc/04inputrc-contents.xml
	[[ $? -eq 0 ]] || failed "Generating inputrc contents xml failed."
	
	echo "Generating dvrc contents xml"
	#gen_contents_xml 0 "d" $dv_app_dir/Contents/Resources/dvrc > davinci.pmdoc/03dvrc-contents.xml
	gen_contents_xml 0 "d" $dir/components/dvrc > davinci.pmdoc/03dvrc-contents.xml
	[[ $? -eq 0 ]] || failed "Generating dvrc contents xml failed."
	
	echo "Generating dvlib contents xml"
	# gen_contents_xml 0 "d" $source/library > davinci.pmdoc/02dvlib-contents.xml
	mkdir $dir/tmp_dvlib
	cp -r $source/library $dir/tmp_dvlib/.
	[[ $? -eq 0 ]] || failed "Unable to copy davinci library."
	find $dir/tmp_dvlib -name '.svn' -prune -type d -exec rm -rf {} \;
	gen_contents_xml 0 "d" $dir/tmp_dvlib/library > dvlib.pmdoc/01dvlib-contents.xml
	[[ $? -eq 0 ]] || failed "Generating dvlib contents xml failed."
	
	echo "Updating dvlib xml file"
	lib_version=`svn info $source/library/. | sed '/^Last Changed Date:/!d;s/^[^:]*: //;s/ .*$//'`
	[[ $? -eq 0 ]] || lib_version=`date +'%Y-%m-%d'`
	xmlstarlet ed -u '/pkgref/config/version' -v "${lib_version}" davinci.pmdoc/02dvlib.xml > /tmp/x.xml && mv /tmp/x.xml davinci.pmdoc/02dvlib.xml
	[[ $? -eq 0 ]] || failed "Updating 02dvlib.xml with version/release failed."

	echo "Updating davinci xml file"
	xmlstarlet ed -u '/pkgref/config/version' -v "${version}-${release}" davinci.pmdoc/01davinci.xml > /tmp/x.xml && mv /tmp/x.xml davinci.pmdoc/01davinci.xml
	[[ $? -eq 0 ]] || failed "Updating 01davinci.xml with version/release failed."
	
	echo "Updating index file with OS version"
	xmlstarlet ed -u '/pkmkdoc/requirements/requirement[1]/@value' -v "${tgt_os}" davinci.pmdoc/index.xml > /tmp/x.xml && mv /tmp/x.xml davinci.pmdoc/index.xml
	[[ $? -eq 0 ]] || failed "Updating index.xml with target OS (low) failed."
	xmlstarlet ed -u '/pkmkdoc/requirements/requirement/message' -v "This release will work on ${tgt_os} or higher" davinci.pmdoc/index.xml > /tmp/x.xml && mv /tmp/x.xml davinci.pmdoc/index.xml
	[[ $? -eq 0 ]] || failed "Updating index.xml with target OS message failed."

	echo "Updating Info.plist file with davinci version"
	info_plist="${dv_app_dir}/Contents/Info.plist"
	xmlstarlet ed -u '//dict/string[text()="9.99"]' -v "${major}.${minor}-${release}" ${info_plist} > /tmp/x.xml && mv /tmp/x.xml ${info_plist}
	[[ $? -eq 0 ]] || failed "Updating Info.plist with davinci version."

	#build the davinci package
	echo "Building davinci-${version}-${release}-${tgt_os}-${tgt_arch} package"
	$PKGMKR --doc davinci.pmdoc --out /tmp/davinci-${version}-${release}-${tgt_os}-${tgt_arch}.mpkg --verbose
	[[ $? -eq 0 ]] || failed "Failed to build davinci package."

	#make the davinci disk image
	echo "Building davinci disk image as $dir/davinci-${version}-${release}-${tgt_os}-${tgt_arch}.dmg"
	rm -f $dir/davinci-${version}-${release}.dmg
	hdiutil create -srcdir /tmp/davinci-${version}-${release}-${tgt_os}-${tgt_arch}.mpkg $dir/davinci-${version}-${release}-${tgt_os}-${tgt_arch}.dmg
	[[ $? -eq 0 ]] || failed "Failed to build davinci disk image."

	# Cleanup
	#rm -rf ${dv_app_dir}
	rm -rf $dir/tmp_dvlib
fi

if [ "`echo $packages | grep dvlib`" ]; then
	##### library package
	echo '**** Building dvlib'

	echo "Generating dvlib contents xml"
	mkdir $dir/tmp_dvlib
	cp -r $source/library $dir/tmp_dvlib/.
	[[ $? -eq 0 ]] || failed "Unable to copy davinci library."
	find $dir/tmp_dvlib -name '.svn' -prune -type d -exec rm -rf {} \;
	gen_contents_xml 0 "d" $dir/tmp_dvlib/library > dvlib.pmdoc/01dvlib-contents.xml
	[[ $? -eq 0 ]] || failed "Generating dvlib contents xml failed."
	
	echo "Updating dvlib xml file"
	lib_version=`svn info $source/library/. | sed '/^Last Changed Date:/!d;s/^[^:]*: //;s/ .*$//'`
	[[ $? -eq 0 ]] || lib_version=`date +'%Y-%m-%d'`
	xmlstarlet ed -u '/pkgref/config/version' -v "${lib_version}" dvlib.pmdoc/01dvlib.xml > /tmp/x.xml && mv /tmp/x.xml dvlib.pmdoc/01dvlib.xml
	[[ $? -eq 0 ]] || failed "Updating 01dvlib.xml with version/release failed."

	#make library package
	echo "Building dvlib-${lib_version} package"
	$PKGMKR --doc dvlib.pmdoc --out /tmp/dvlib-${lib_version}.mpkg --verbose
	[[ $? -eq 0 ]] || failed "Failed to build dvlib package."

	#make library package disk image
	echo "Building dvlib disk image as $dir/dvlib-${lib_version}.dmg"
	rm -f $dir/dvlib-${lib_version}.dmg
	hdiutil create -srcdir /tmp/dvlib-${lib_version}.mpkg $dir/dvlib-${lib_version}.dmg
	[[ $? -eq 0 ]] || failed "Failed to build dvlib disk image."

	rm -rf $dir/tmp_dvlib
fi


exit 0

