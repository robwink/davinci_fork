This directory works but it is far from the correct/best/easiest way to build
an OS X application.

Robert Winkler notes (semi-MSFF specific)
=========================================

After cloning the 10.11 - DEV machine, you have to

1.) agree to the xcode license (just run git or something on the command line and it'll make you go through the steps.

2.) install the latest XQuartz and log out/in to use it.

3.) install Macports and through that xmlstarlet.  Or you could get it from
fink or a similar 3rd party mac package manager (but you may have to tweak the
build script, ie xmlstarlet is xml from fink).
"Third-party projects such as Homebrew, Fink, MacPorts and pkgsrc provide pre-compiled or pre-formatted packages."

4.) Finally you have to get PackageMaker since that crap is years gone from OS X.

"
Packagemaker has been removed from XCode since XCode 4. It is however available inside the "Auxiliary Tools for XCode - Late July 2012" dmg which can be downloaded from Apple's developer tools downloads section here.
"

Apparently the way you're supposed to do it is with these "new" tools.
http://stackoverflow.com/questions/11487596/making-os-x-installer-packages-like-a-pro-xcode-developer-id-ready-pkg

If you clone my 10.11 DEV machine (with rsw suffix) you can skip all the very
annoying steps listed above, but it's important to know the crap we're dealing
with on mac.

I had to recreate my clone and go through those steps again the week of Nov. 7th
because my VM was corrupted somehow and wouldn't even boot.  I did have a
bunch of other things installed to make working on mac only insanely unbearable
and not "death by 10000 papercuts".

Things I remember installing for sanity:

http://www.trankynam.com/xtrafinder/
for things like tabbed browsing and "new terminal here" etc. honestly how can
Apple justify not having those features?  Idiotic

http://www.iterm2.com/
For a much better terminal

There were other things that I'm forgetting.  And in case you think I'm being
unfair to mac, I install a similar (much better) explorer extension in windows
for tabbed file browsing and much more and I install MSYS2, git-bash etc. for
unix style terminal (and package manager) experience.

That's what makes these lacks on Mac so frustrating.  It already is Unix and
yet it's terminal is pathetic and there's no package manager.  Oh well.



Old notes on the crappy build process
=====================================

Notes:
* libtool and DESTDIR don't work well together
* As a consequence, the entire distribution is built in the /Applications/davinci.app
  directory and packaged using a PackageMaker pmdoc spec-file.

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

