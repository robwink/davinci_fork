$ !
$ ! VMS procedure to compile and link modules for plot widgets
$ !
$ SET NOVERIFY
$ ON ERROR THEN EXIT
$ ! COMPILE := CC/DEBUG/NOOPTIMIZE
$ ! COMPILE := CC/VAXC
$ COMPILE := CC/STANDARD=VAXC
$ ! For some systems: COMPILE := CC /PREFIX_LIBRARY_ENTRIES=ALL_ENTRIES
$ DEFINE SYS DECC$LIBRARY_INCLUDE
$ DEFINE XM DECW$INCLUDE
$ DEFINE X11 DECW$INCLUDE
$ !
$ SET VERIFY
$ COMPILE demoUtils.c
$ COMPILE 2DHistDemo.c
$ COMPILE 2DHistSimple.c
$ COMPILE 3DScatDemo.c
$ COMPILE 3DScatSimple.c
$ COMPILE cellDemo.c
$ COMPILE cellSimple.c
$ COMPILE h1DDemo.c
$ COMPILE h1DSimple.c
$ COMPILE scatDemo.c
$ COMPILE scatSimple.c
$ COMPILE xyDemo.c
$ COMPILE xySimple.c
$ !
$ @LNKEXAMPLES
