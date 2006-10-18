#
# Makefile for plot widgets and examples
#
# target should be machine type: sgi, ultrix, ibm, sunos, solaris, dec
#
# Builds an intermediate library in util directory, then builds
# the nedit executable in the source directory
#

all:
	@echo "Plot widgets targets:  sgi         ultrix         ibm        linux"
	@echo "                       sunos       solaris        dec"

sgi:
	(cd util; make -f Makefile.sgi libNUtil.a)
	(cd plot_widgets; make -f Makefile.sgi libPlotW.a)
	(cd examples; make -f Makefile.sgi all)

# HP may require a patched rint function
#hp:
#	(cd util; make -f Makefile.hp libNUtil.a)
#	(cd plot_widgets; make -f Makefile.hp libPlotW.a)
#	(cd examples; make -f Makefile.hp all)

ultrix:
	(cd util; make -f Makefile.ultrix libNUtil.a)
	(cd plot_widgets; make -f Makefile.ultrix libPlotW.a)
	(cd examples; make -f Makefile.ultrix all)

ibm:
	(cd util; make -f Makefile.ibm libNUtil.a)
	(cd plot_widgets; make -f Makefile.ibm libPlotW.a)
	(cd examples; make -f Makefile.ibm all)

linux:
	(cd util; make -f Makefile.linux libNUtil.a)
	(cd plot_widgets; make -f Makefile.linux libPlotW.a)
	(cd examples; make -f Makefile.linux all)

sunos:
	(cd util; make -f Makefile.sunos libNUtil.a)
	(cd plot_widgets; make -f Makefile.sunos libPlotW.a)
	(cd examples; make -f Makefile.sunos all)

solaris:
	(cd util; make -f Makefile.solaris libNUtil.a)
	(cd plot_widgets; make -f Makefile.solaris libPlotW.a)
	(cd examples; make -f Makefile.solaris all)

dec:
	(cd util; make -f Makefile.dec libNUtil.a)
	(cd plot_widgets; make -f Makefile.dec libPlotW.a)
	(cd examples; make -f Makefile.dec all)
