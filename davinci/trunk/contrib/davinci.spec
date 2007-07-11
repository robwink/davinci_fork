##
##  davinci spec -- Mars Space Flight Facility RPM Package Specification
##  
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

#   package information
Name:         davinci
Summary:      Davinci - A tool to manipulate and view various types of data, developed in Mars Space Flight Facility
URL:          http://davinci.asu.edu
Vendor:       Mars Space Flight Facility at Arizona State University
Packager:     Betim Deva <betim@asu.edu>  
Distribution: Fedora Core 4 (MSFF)
Group:        Development/Languages/Davinci
License:      GPLv2
Version:      1.68
Release:      1

#   list of sources
Source:      ftp://ftp.mars.asu.edu/pub/software/davinci/%{name}-%{version}.tar.gz

#   build information
#BuildPreReq:  make, gcc
BuildRoot: %{_tmppath}/%{name}-%{version}-buildroot
Requires:	hdf5, readline, zlib
#Provides:     MTA, smtpdaemon
#Obsoletes:    sendmail
#Conflicts:    exim, postfix, qmail

%description
	Davinci is an interpreted language that looks and feels a lot like C, 
	but has additional vector oriented features that make working with blocks 
	of data a lot easier. This makes davinci well suited for use as a data 
	processing tool, allowing symbolic and mathematical manipulation of 
	hyperspectral data for imaging spectroscopy applications. 

%prep

%setup -q

%build
./configure --prefix=/usr  --with-help=%{_libdir}/davinci/dv.gih
make

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p -m 755 $RPM_BUILD_ROOT%{_bindir}
mkdir -p -m 755 $RPM_BUILD_ROOT%{_libdir}
make install DESTDIR=$RPM_BUILD_ROOT

%clean
rm -rf $RPM_BUILD_ROOT

%files
 %defattr(-,root,root)
 %{_bindir}/davinci
 %{_libdir}/*

%changelog
* Tue Jul 10 2007 Betim Deva <betim@asu.edu> 1.6.8-1
- Created the SPEC file

