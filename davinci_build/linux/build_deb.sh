#!/bin/sh
# Script: Ubuntu/Debian deb builder for Davinci
# Author: Christopher Edwards (cedwards@mars.asu.edu)
#
# Build a davinci deb file. This will setup a neccessary environment for 
# davinci deb based on the latest davinci version, invoke dpkg,
# and name the deb file with a release version
# 
# It is necessary to run this script on a Debian/Ubuntu machine, and run
# from the directory where the script lives.
#
# If a davinci executeable exists, a compile will not happen rather, 
# just the debian package will be made
#

failed() {
    echo $1
    exit 1
}


#set -x
release=1
force_rebuild=1

start_dir=`pwd`

#get the davinci src directory and make a clean install directory
davinci_src=`pwd`/../..

deb_dir=${davinci_src}/deb/
if [ -e $deb_dir ]; then
    rm -rf $deb_dir
fi

#get and check the version
version=`grep 'davinci ' ${davinci_src}/version.h | sed -e 's/\(.*\) \(.*\)";/\2/'`
if [ "${version}" = "" ]; then
    echo "Davinci version was not found"
    exit
fi

name=davinci
mach=`uname -m`
arch=`uname -p`
case "$mach" in
    i*)
        arch=i386
        lib=lib
        ;;
    x86_64)
        arch=amd64
        lib=lib64
        ;;
    *)
        echo "Unknown architecture"
        exit 1
        ;;
esac

#setup the debian package name
deb=${name}-${version}-${release}.${arch}.deb

#if a davinci executeable exists then don't compile just make the package
if [ ! -e $davinci_src/davinci -o $force_rebuild -ne 0 ]
    then 
    
    cd $davinci_src
    make clean
    
    #make the png lib first to use local copy
    cd $davinci_src/iomedley/libpng-1.2.3
    ./configure CFLAGS=-fPIC CXXFLAGS=-fPIC
    make
    cd $davinci_src   

    #configure with appropriate settings
    ./configure --prefix=/usr --disable-jbig --disable-libisis --with-cfitsio=/usr --with-readline=/usr --with-viewer=/usr/bin/display --with-modpath=/usr/${lib}/${name} --with-help=/usr/share/davinci/docs/dv.gih CFLAGS=-fPIC CXXFLAGS=-fPIC
    [ $? -eq 0 ] || failed "Failure configuring davinci"

    #make the binary
    make clean
    make
    [ $? -eq 0 ] || failed "davinci build failed"
fi

#check to make sure davicni compiled
[ -e $davinci_src/davinci ] || failed "davinci executeable could not be made"

cd $davinci_src

#make the directory for the path with the right control file etc
mkdir -p -m 755 $deb_dir/usr/bin
mkdir -p -m 775 $deb_dir/usr/lib
mkdir -p -m 775 $deb_dir/usr/include
mkdir -p -m 775 $deb_dir/usr/share/davinci

#copy the DEBIAN control files to the package directory
tar -C contrib --exclude '.svn' --exclude 'CVS' -cf - DEBIAN | tar -C $deb_dir -xf -
[ $? -eq 0 ] || failed "Failed to copy control files to package directory"

#install the components to the tmp dir
make install DESTDIR=$deb_dir
[ $? -eq 0 ] || failed "make install failed"

tar -C $davinci_src --exclude '.svn' --exclude 'CVS' -cf - library | tar -C $deb_dir/usr/share/davinci/ -xf -
[ $? -eq 0 ] || failed "Copying library to package directory"

#update the DEBIAN control file with the platform/version information
ctrl_file=$deb_dir/DEBIAN/control
chmod 755 $deb_dir/DEBIAN/control
tmp_ctrl_file=/tmp/control.$$
size_in_bytes=`du -bs $deb_dir 2>/dev/null | awk '{ print $1 }'`
size_in_bytes2=`echo "$size_in_bytes / 1024" | bc `

sed "s/^\(Version:\).*$/\1 ${version}/;s/^\(Architecture:\).*$/\1 ${arch}/;s/^\(Installed-Size:\).*$/\1 ${size_in_bytes2}/" < $ctrl_file > $tmp_ctrl_file && mv $tmp_ctrl_file $ctrl_file
[ $? -eq 0 ] || failed "Unable to update version information in control file."

# TODO update dependencies using either dpkg-gencontrol or:
# objdump -p .libs/davinci .libs/libdavinci.so modules/thm/.libs/thm.so modules/cse/.libs/cse.so modules/kjn/.libs/kjn.so | grep NEEDED | awk '{ print $2 }' | sort -u | xargs -n1 dpkg -S | awk -F: '{ print $1 }' | sort -u

# TODO add version log to the control file
# TODO add license to the control file

#change the 
sudo chown -R root:root $deb_dir/

#build the debian package
dpkg -b $deb_dir ${start_dir}/$deb
[ $? -eq 0 ] || failed "Building davinci package using dpkg"

# everything built successfully
exit 0

