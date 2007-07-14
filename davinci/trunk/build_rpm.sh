#!/bin/sh
# Script: Fedora/Redhat rpm builder for Davinci
# Author: Betim Deva (betim@asu.edu)
#
# Build a davinci rpm set. This will setup a neccessary environment for 
# davinci rpm based on the latest davinci version, invoke rpmbuild,
# and copy over the rpm files to the davinci source
# 
#
#
set -x

##Get davinci source absolute path
davinci_src=`echo $0 |  sed -e 's|\(.*\)\/\(.*\)$|\1|'`
if [ "${davinci_src}" = "." ]
then
	davinci_src=`pwd`
else
	davinci_src=`pwd`/${davinci_src}
fi


version=`grep 'daVinci Version #' version.h | sed -e 's/\(.*\)#\(.*\)";/\2/'`



if [ "${version}" = "" ] 
then
    echo "Davinci version was not found"
    exit
fi



##Remove the previous rpms
rm -f ${davinci_src}/*.rpm


r=${HOME}/rpm
##Create a macro file if it does not exist
if [ ! -f "${HOME}/.rpmmacros" ]
then
	echo "%_topdir  ${r}" >> ${HOME}/.rpmmacros
	echo "%_tmppath ${r}/tmp" >> ${HOME}/.rpmmacros

##Otherwise use whats in the .rpmmacros
else	
	r=`cat ~/.rpmmacros | grep "%_topdir" | sed 's|^\(.\+\)\([ \t]\+\)\(.\+\)$|\3|'`
fi

o=davinci-${version}
s=${r}/SOURCES
spec=${o}.spec


## Create rpm dirs (if they don't exist)
mkdir -p ${r}
mkdir -p ${r}/SOURCES
mkdir -p ${r}/SRPMS
mkdir -p ${r}/SPECS
mkdir -p ${r}/BUILD
mkdir -p ${r}/RPMS

## Remove previous files (if they exist)
rm -rf ${r}/SOURCES/${o}*
rm -rf ${r}/SRPMS/${o}*
rm -rf ${r}/SPECS/${o}*
rm -rf ${r}/BUILD/${o}*
rm -rf ${r}/RPMS/${o}*



mkdir ${s}/${o}
cp -rf ${davinci_src}/* ${s}/${o} 


base_spec=${davinci_src}/contrib/davinci.spec
#put an appropriate version in the destination SPEC file
`sed ${base_spec} -e "s/Version:.*/Version:\t\${version}/" > ${r}/SPECS/${spec}`


#Go to Sources
cd ${s}
#Tar the source
tar -cf ${o}.tar ${o}
gzip ${o}.tar


# Now build the binaries and the rpms.
rpmbuild -ba ${r}/SPECS/${spec}

# Copy back to the dacinci directory
cp -f  ${r}/SRPMS/*.rpm ${davinci_src}
cp -f  ${r}/RPMS/i386/*.rpm ${davinci_src}
