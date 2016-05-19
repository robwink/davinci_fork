#!/bin/bash
# Script: Fedora/Redhat rpm builder for Davinci
# Author: Betim Deva (betim@asu.edu)
#
# Build a davinci rpm set. This will setup a neccessary environment for 
# davinci rpm based on the latest davinci version, invoke rpmbuild,
# and copy over the rpm files to the davinci source
# 
#
#

function failed() {
	echo $1
	exit 1
}

echo "PATH:$PATH"
set -x

##Get davinci source absolute path



# script_dir0=`echo $0 |  sed -e 's|\(.*\)\/\(.*\)$|\1|'`
script_dir0=`dirname $0`


if [ "${script_dir0}" = "." ]
then
	script_dir=`pwd`
else
	script_dir=${script_dir0}
fi

davinci_src=${script_dir}/../..

version=`grep 'davinci ' ${davinci_src}/version.h | sed -e 's/\(.*\) \(.*\)";/\2/'`



[ "${version}" = "" ]  && failed "Davinci version was not found"


##Remove the previous rpms
rm -f ${davinci_src}/*.rpm


r=${HOME}/rpm
##Create a macro file if it does not exist
if [ ! -f "${HOME}/.rpmmacros" ]
then
	echo "%_topdir  ${r}" >> ${HOME}/.rpmmacros
	echo "%_tmppath ${r}/tmp" >> ${HOME}/.rpmmacros
	[[ $? -eq 0 ]] || failed "Unable to create ${HOME}/.rpmmacros."

##Otherwise use whats in the .rpmmacros
else	
	r=`cat ${HOME}/.rpmmacros | grep "%_topdir" | sed 's|^\(.\+\)\([ \t]\+\)\(.\+\)$|\3|'`
	[[ $? -eq 0 ]] || failed "Unable to get '%_topdir' from ${HOME}/.rpmmacros."
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
[[ $? -eq 0 ]] || failed "Unable to create rpm dirs in ${r}."

## Remove previous files (if they exist)
rm -rf ${r}/SOURCES/${o}*
rm -rf ${r}/SRPMS/${o}*
rm -rf ${r}/SPECS/${o}*
rm -rf ${r}/BUILD/${o}*
rm -rf ${r}/RPMS/${o}*



mkdir ${s}/${o}
[[ $? -eq 0 ]] || failed "Unable to create dir ${s}/${o}."
# cp -rf ${davinci_src}/* ${s}/${o} 
tar -C ${davinci_src} --exclude '\.svn' -hcf - . | tar -C ${s}/${o} -xf - 
[[ $? -eq 0 ]] || failed "Unable to copy davinci source from ${davinci_src} into dir ${s}/${o}."


base_spec=${davinci_src}/contrib/davinci.spec
#put an appropriate version in the destination SPEC file
`sed ${base_spec} -e "s/Version:.*/Version:\t\${version}/" > ${r}/SPECS/${spec}`
[[ $? -eq 0 ]] || failed "Unable to update ${r}/SPECS/${spec} from ${base_spec}."


#Go to Sources
cd ${s}
#Tar the source
tar --exclude='.svn' -zhcf ${o}.tar.gz ${o}
[[ $? -eq 0 ]] || failed "Unable to tar sources in ${s}/${o}."


# Now build the binaries and the rpms.
rpmbuild -ba ${r}/SPECS/${spec}
[[ $? -eq 0 ]] || failed "Unable to build RPM using ${r}/SPECS/${spec}."

# Copy back to the dacinci directory
cp -f  ${r}/SRPMS/*.rpm ${script_dir} || failed "Unable to copy ${r}/SRPMS/*.rpm to ${script_dir}"
cp -f  ${r}/RPMS/`uname -i`/*.rpm ${script_dir} || failed "Unablt to copy ${r}/RPMS/`uname -i`/*.rpm to ${script_dir}"

# Cleanup after ourselves
rm -rf ${r}/SOURCES/${o}*
rm -rf ${r}/SRPMS/${o}*
rm -rf ${r}/SPECS/${o}*
rm -rf ${r}/BUILD/${o}*
rm -rf ${r}/RPMS/${o}*

exit 0
