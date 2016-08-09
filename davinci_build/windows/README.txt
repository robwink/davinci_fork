

If you're setting it up on a fresh windows system ...

Download and install MSYS2, follow the directions on the website for updating
pacman/packages
https://msys2.github.io/


good reference for pacman (and other package managers)
https://wiki.archlinux.org/index.php/Pacman/Rosetta

install the following (not all are strictly necessary but meh, and some
probably pull in helpful/necessary stuff):
=====================
git
subversion
make
gcc
python
python2
pkgfile (for finding what packages provide a given file)

libxml2-devel
libreadline-devel
libcurl-devel

# for 64 bit, otherwise replace x86_64 with i686
mingw-w64-x86_64-python2
mingw-w64-x86_64-python3

mingw-w64-x86_64-make
mingw-w64-x86_64-gcc
mingw-w64-x86_64-clang
mingw-w64-x86_64-pkg-config


mingw-w64-x86_64-libxml2
mingw-w64-x86_64-libreadline
mingw-w64-x86_64-hdf5
mingw-w64-x86_64-curl

...


clone (or checkout) the repo
cd to davinci_build/windows/dependencies/cfitsio

./configure && make install

back to source root dir

=====================================
START HERE on monopoly2.mars.asu.edu
everything above has already been done
======================================

in source root

./configure CXXFLAGS='-fpermissive' && make

If you're on a VM that can take 15 minutes

cd davinci_build/windows
./make_win.sh ../..

copy the exe dll and modules/ to the install directory
(ie the folder contained what would go in C:/Program Files/Davinci)

On monopoly2 that would be
/c/msys64/home/rswinkle/davinci_win/


cd to parent of install directory and copy the nsi script there
run NSIS on the script (after changing the defines at the top)

Choose LZMA since it's the best compression




