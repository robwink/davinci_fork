##
##  davinci spec -- Mars Space Flight Facility RPM Package Specification
##  This is a base SPEC file for davinci. This file may be modified 
##  and written to a new file, if the rpms are built from the build_rom.sh
##  script in order to modify the Version field.
##
##  Permission to use, copy, modify, and distribute this software for
##  any purpose with or without fee is hereby granted, provided that
##  the above copyright notice and this permission notice appear in all
##  copies.
##
##  THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
##  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
##  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
##  IN NO EVENT SHALL THE AUTHORS AND COPYRIGHT HOLDERS AND THEIR
##  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
##  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
##  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
##  USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
##  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
##  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
##  OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
##  SUCH DAMAGE.
##
AutoReqProv: no
#   package information
Name:         davinci
Summary:      Davinci - A tool to manipulate and view various types of data, developed in Mars Space Flight Facility
URL:          http://davinci.asu.edu
Vendor:       Mars Space Flight Facility at Arizona State University
Packager:     Davinci Devs <davinci-dev@mars.asu.edu>
Distribution: CentOS 6 (MSFF)
Group:        Applications/Science
License:      GPLv2
Version:      2.17
Release:      3

# This was generated using the following process:
# svn checkout http://oss.mars.asu.edu/svn/davinci/davinci/tags/dv_2_17/ davinci-2.17
# tar czf davinci-2.17.tar.gz davinci-2.17 --exclude=*.svn
Source:      %{name}-%{version}.tar.gz

# This is needed to fix undefined references to lzma_ functions from libdavinci.so
# Patch1: davinci-autoconf-lzma.patch

#   build information
BuildRequires: automake
BuildRequires: autoconf
BuildRequires: make
BuildRequires: gcc
BuildRequires: gcc-c++
BuildRequires: hdf5
BuildRequires: hdf5-devel 
BuildRequires: cfitsio
BuildRequires: cfitsio-devel
BuildRequires: readline
BuildRequires: readline-devel
BuildRequires: zlib
BuildRequires: zlib-devel
BuildRequires: curl-devel
BuildRequires: lzma-devel
BuildRequires: libxml2-devel
BuildRoot: %{_tmppath}/%{name}-%{version}-buildroot

Requires: gnuplot
Requires: hdf5
Requires: hdf5-devel
Requires: cfitsio
Requires: readline
Requires: zlib
Requires: libxml2
Requires: libXmu-devel
Requires: libtool-ltdl-devel
Requires: openmotif-devel
Requires: compat-libf2c-34
#Requires: isis2

Prefix: %_prefix

%description
	Davinci is an interpreted language that looks and feels a lot like C,
	but has additional vector oriented features that make working with blocks
	of data a lot easier. This makes davinci well suited for use as a data
	processing tool, allowing symbolic and mathematical manipulation of
	hyperspectral data for imaging spectroscopy applications.

%prep

%setup -q
#%patch1 -p1
#autoconf -f

%build
#Iomedley
cd iomedley
./configure --prefix=%{_prefix} --libdir=%{_libdir}
make 
cd ../

#Davinci
./configure --prefix=%{_prefix} --libdir=%{_libdir} --with-libxml2=/usr/include  --with-viewer=/usr/bin/display \
--with-modpath=%{_libdir}/%{name} --with-help=%{_datadir}/%{name}/docs/dv.gih --with-cfitsio=/usr/include/cfitsio --enable-libisis=/mars/common/isis2_64/isisr

make

%define __spec_install_post %{nil}
%define debug_package %{nil}

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p -m 755 $RPM_BUILD_ROOT%{_bindir}
mkdir -p -m 755 $RPM_BUILD_ROOT%{_libdir}
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
 %defattr(-,root,root)
 %{_bindir}/%{name}
 %{_libdir}/libdavinci*
 %{_libdir}/libiomedley*
 %{_libdir}/%{name}*
 %{_datadir}/%{name}*
 %{_includedir}/%{name}*

%post
##To avoid SELINUX  security message (maybe there is a better solution)
chcon -f -t textrel_shlib_t %{_libdir}/libdavinci* > /dev/null 2>&1 || /bin/true

%changelog
* Mon Jun 22 2015 Nick Piacentine <npiace@mars.asu.edu> 2.17-3
- Rebuilt with cleaned lzma patch.
 
* Wed Apr 15 2015 Nick Piacentine <npiace@mars.asu.edu> 2.17-2
- Rebuilt with libxml2 and libisis support

* Tue Apr 14 2015 Nick Piacentine <npiace@mars.asu.edu> 2.17-1
- Rebuilt for davinci-2.17

* Tue Oct 14 2014 Nick Piacentine <npiace@mars.asu.edu> 2.16-1
- Updated SPEC file to use BuildRequires
- Added lzma-devel requirement for build.
- Added patch to build with lzma

* Tue Jul 10 2007 Betim Deva <betim@asu.edu> 1.6.8-1
- Created the SPEC file

