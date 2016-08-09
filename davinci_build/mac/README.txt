This directory works, but it is still a work-in-progress from its source, which is the
"mac" directory on the same level.

Notes:
* libtool and DESTDIR don't work well together
* As a consequence, the entire distribution is built in the /Applications/davinci.app
  directory and packaged using a PackageMaker pmdoc spec-file.
* The pmdoc files produced by PackageMaker are not formatted. One can use the
  "xml" tool available from "fink" and use "xml fo" to format the files.

Here is how the build script (build_mac.sh) works:
1. The build script copies a davinci.app template from the components directory
   into /Applications/davinci.app. The template comes with ImageJ as viewer instead
   of assuming "xv" being available on the system. Examples and libraries are also
   copied at this point.
2. It then builds the following dependencies and installs them in the MacOS/Resources
   directory under /Applications/davinci.app:
   a) hdf5
   b) cfitsio
   c) gnuplot
3. It then builds iomedley (which is technically also a dependency).
4. It then builds davinci using the dependencies already built in step 2 and 3.
   This ensures that davinci is not dependent upon any external libraries other than
   what is shipped with MacOS or whatever we chose to build explicitly (without 
   any fink dependencies).
5. Update the davinci.pmdoc PackageMaker description of the davinci meta-package
   (containing davinci, dvrc and inputrc packages) from the contents of 
   /Applications/davinci.app and the davinci source code.
6. Package the following into a davinci package by using the davinci.pmdoc:
   a) /Applications/davinci.app
   b) components/dvrc (with install scripts in components/extras-dvrc)
   c) components/inputrc (with install scripts in components/extras-inputrc)
7. Build a disk image out of the package.
8. Cleanup: Remove the /Applications/davinci.app directory.

