$ !
$ ! DCL link procedure for widget example programs
$ !
$ SET VERIFY
$ LINK 2DHistDemo, demoutils, EXAMPLES_OPTIONS_FILE/OPT, [-.plot_widgets]libPlotW/lib, [-.util]vmsUtils/lib, libUtil/lib
$ LINK 2DHistSimple, demoutils, EXAMPLES_OPTIONS_FILE/OPT, [-.plot_widgets]libPlotW/lib, [-.util]vmsUtils/lib, libUtil/lib
$ LINK 3DScatDemo, demoutils, EXAMPLES_OPTIONS_FILE/OPT, [-.plot_widgets]libPlotW/lib, [-.util]vmsUtils/lib, libUtil/lib
$ LINK 3DScatSimple, demoutils, EXAMPLES_OPTIONS_FILE/OPT, [-.plot_widgets]libPlotW/lib, [-.util]vmsUtils/lib, libUtil/lib
$ LINK cellDemo, demoutils, EXAMPLES_OPTIONS_FILE/OPT, [-.plot_widgets]libPlotW/lib, [-.util]vmsUtils/lib, libUtil/lib
$ LINK cellSimple, demoutils, EXAMPLES_OPTIONS_FILE/OPT, [-.plot_widgets]libPlotW/lib, [-.util]vmsUtils/lib, libUtil/lib
$ LINK h1DDemo, demoutils, EXAMPLES_OPTIONS_FILE/OPT, [-.plot_widgets]libPlotW/lib, [-.util]vmsUtils/lib, libUtil/lib
$ LINK h1DSimple, demoutils, EXAMPLES_OPTIONS_FILE/OPT, [-.plot_widgets]libPlotW/lib, [-.util]vmsUtils/lib, libUtil/lib
$ LINK scatDemo, demoutils, EXAMPLES_OPTIONS_FILE/OPT, [-.plot_widgets]libPlotW/lib, [-.util]vmsUtils/lib, libUtil/lib
$ LINK scatSimple, demoutils, EXAMPLES_OPTIONS_FILE/OPT, [-.plot_widgets]libPlotW/lib, [-.util]vmsUtils/lib, libUtil/lib
$ LINK xyDemo, demoutils, EXAMPLES_OPTIONS_FILE/OPT, [-.plot_widgets]libPlotW/lib, [-.util]vmsUtils/lib, libUtil/lib
$ LINK xySimple, demoutils, EXAMPLES_OPTIONS_FILE/OPT, [-.plot_widgets]libPlotW/lib, [-.util]vmsUtils/lib, libUtil/lib
$ ON WARNING THEN EXIT
$ SET NOVERIFY
