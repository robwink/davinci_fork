# Microsoft Developer Studio Generated NMAKE File, Based on davinci.dsp
!IF "$(CFG)" == ""
CFG=davinci - Win32 Debug
!MESSAGE No configuration specified. Defaulting to davinci - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "davinci - Win32 Release" && "$(CFG)" !=\
 "davinci - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "davinci.mak" CFG="davinci - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "davinci - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "davinci - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "davinci - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\davinci.exe"

!ELSE 

ALL : "$(OUTDIR)\davinci.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\apifunc.obj"
	-@erase "$(INTDIR)\array.obj"
	-@erase "$(INTDIR)\dct.obj"
	-@erase "$(INTDIR)\error.obj"
	-@erase "$(INTDIR)\ff.obj"
	-@erase "$(INTDIR)\ff_ascii.obj"
	-@erase "$(INTDIR)\ff_avg.obj"
	-@erase "$(INTDIR)\ff_bbr.obj"
	-@erase "$(INTDIR)\ff_cluster.obj"
	-@erase "$(INTDIR)\ff_convolve.obj"
	-@erase "$(INTDIR)\ff_display.obj"
	-@erase "$(INTDIR)\ff_fft.obj"
	-@erase "$(INTDIR)\ff_gnoise.obj"
	-@erase "$(INTDIR)\ff_gplot.obj"
	-@erase "$(INTDIR)\ff_header.obj"
	-@erase "$(INTDIR)\ff_ifill.obj"
	-@erase "$(INTDIR)\ff_interp.obj"
	-@erase "$(INTDIR)\ff_ix.obj"
	-@erase "$(INTDIR)\ff_load.obj"
	-@erase "$(INTDIR)\ff_moment.obj"
	-@erase "$(INTDIR)\ff_pause.obj"
	-@erase "$(INTDIR)\ff_plplot.obj"
	-@erase "$(INTDIR)\ff_projection.obj"
	-@erase "$(INTDIR)\ff_random.obj"
	-@erase "$(INTDIR)\ff_rgb.obj"
	-@erase "$(INTDIR)\ff_sort.obj"
	-@erase "$(INTDIR)\ff_source.obj"
	-@erase "$(INTDIR)\ff_struct.obj"
	-@erase "$(INTDIR)\ff_text.obj"
	-@erase "$(INTDIR)\ff_version.obj"
	-@erase "$(INTDIR)\ff_vignette.obj"
	-@erase "$(INTDIR)\ff_write.obj"
	-@erase "$(INTDIR)\ff_xfrm.obj"
	-@erase "$(INTDIR)\fft.obj"
	-@erase "$(INTDIR)\fft2f.obj"
	-@erase "$(INTDIR)\fft_mayer.obj"
	-@erase "$(INTDIR)\fit.obj"
	-@erase "$(INTDIR)\help.obj"
	-@erase "$(INTDIR)\init.obj"
	-@erase "$(INTDIR)\io_ascii.obj"
	-@erase "$(INTDIR)\io_aviris.obj"
	-@erase "$(INTDIR)\io_byteswap.obj"
	-@erase "$(INTDIR)\io_ers.obj"
	-@erase "$(INTDIR)\io_goes.obj"
	-@erase "$(INTDIR)\io_grd.obj"
	-@erase "$(INTDIR)\io_imath.obj"
	-@erase "$(INTDIR)\io_isis.obj"
	-@erase "$(INTDIR)\io_lablib3.obj"
	-@erase "$(INTDIR)\io_magic.obj"
	-@erase "$(INTDIR)\io_pnm.obj"
	-@erase "$(INTDIR)\io_specpr.obj"
	-@erase "$(INTDIR)\io_themis.obj"
	-@erase "$(INTDIR)\io_vicar.obj"
	-@erase "$(INTDIR)\lexer.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\map.obj"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\misc.obj"
	-@erase "$(INTDIR)\newfunc.obj"
	-@erase "$(INTDIR)\p.obj"
	-@erase "$(INTDIR)\parser.obj"
	-@erase "$(INTDIR)\pp.obj"
	-@erase "$(INTDIR)\pp_math.obj"
	-@erase "$(INTDIR)\printf.obj"
	-@erase "$(INTDIR)\reserved.obj"
	-@erase "$(INTDIR)\rfunc.obj"
	-@erase "$(INTDIR)\rpos.obj"
	-@erase "$(INTDIR)\scope.obj"
	-@erase "$(INTDIR)\string.obj"
	-@erase "$(INTDIR)\symbol.obj"
	-@erase "$(INTDIR)\system.obj"
	-@erase "$(INTDIR)\ufunc.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\w_readline.obj"
	-@erase "$(OUTDIR)\davinci.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /ML /GX /O2 /I "win32" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "_MBCS" /D "__MSDOS__" /D "LITTLE_E" /D "HAVE_LIBREADLINE"\
 /Fp"$(INTDIR)\davinci.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\Release/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\davinci.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib hdf5.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)\davinci.pdb" /machine:I386 /out:"$(OUTDIR)\davinci.exe"\
 /libpath:"win32" 
LINK32_OBJS= \
	"$(INTDIR)\apifunc.obj" \
	"$(INTDIR)\array.obj" \
	"$(INTDIR)\dct.obj" \
	"$(INTDIR)\error.obj" \
	"$(INTDIR)\ff.obj" \
	"$(INTDIR)\ff_ascii.obj" \
	"$(INTDIR)\ff_avg.obj" \
	"$(INTDIR)\ff_bbr.obj" \
	"$(INTDIR)\ff_cluster.obj" \
	"$(INTDIR)\ff_convolve.obj" \
	"$(INTDIR)\ff_display.obj" \
	"$(INTDIR)\ff_fft.obj" \
	"$(INTDIR)\ff_gnoise.obj" \
	"$(INTDIR)\ff_gplot.obj" \
	"$(INTDIR)\ff_header.obj" \
	"$(INTDIR)\ff_ifill.obj" \
	"$(INTDIR)\ff_interp.obj" \
	"$(INTDIR)\ff_ix.obj" \
	"$(INTDIR)\ff_load.obj" \
	"$(INTDIR)\ff_moment.obj" \
	"$(INTDIR)\ff_pause.obj" \
	"$(INTDIR)\ff_plplot.obj" \
	"$(INTDIR)\ff_projection.obj" \
	"$(INTDIR)\ff_random.obj" \
	"$(INTDIR)\ff_rgb.obj" \
	"$(INTDIR)\ff_sort.obj" \
	"$(INTDIR)\ff_source.obj" \
	"$(INTDIR)\ff_struct.obj" \
	"$(INTDIR)\ff_text.obj" \
	"$(INTDIR)\ff_version.obj" \
	"$(INTDIR)\ff_vignette.obj" \
	"$(INTDIR)\ff_write.obj" \
	"$(INTDIR)\ff_xfrm.obj" \
	"$(INTDIR)\fft.obj" \
	"$(INTDIR)\fft2f.obj" \
	"$(INTDIR)\fft_mayer.obj" \
	"$(INTDIR)\fit.obj" \
	"$(INTDIR)\help.obj" \
	"$(INTDIR)\init.obj" \
	"$(INTDIR)\io_ascii.obj" \
	"$(INTDIR)\io_aviris.obj" \
	"$(INTDIR)\io_byteswap.obj" \
	"$(INTDIR)\io_ers.obj" \
	"$(INTDIR)\io_goes.obj" \
	"$(INTDIR)\io_grd.obj" \
	"$(INTDIR)\io_imath.obj" \
	"$(INTDIR)\io_isis.obj" \
	"$(INTDIR)\io_lablib3.obj" \
	"$(INTDIR)\io_magic.obj" \
	"$(INTDIR)\io_pnm.obj" \
	"$(INTDIR)\io_specpr.obj" \
	"$(INTDIR)\io_themis.obj" \
	"$(INTDIR)\io_vicar.obj" \
	"$(INTDIR)\lexer.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\map.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\misc.obj" \
	"$(INTDIR)\newfunc.obj" \
	"$(INTDIR)\p.obj" \
	"$(INTDIR)\parser.obj" \
	"$(INTDIR)\pp.obj" \
	"$(INTDIR)\pp_math.obj" \
	"$(INTDIR)\printf.obj" \
	"$(INTDIR)\reserved.obj" \
	"$(INTDIR)\rfunc.obj" \
	"$(INTDIR)\rpos.obj" \
	"$(INTDIR)\scope.obj" \
	"$(INTDIR)\string.obj" \
	"$(INTDIR)\symbol.obj" \
	"$(INTDIR)\system.obj" \
	"$(INTDIR)\ufunc.obj" \
	"$(INTDIR)\w_readline.obj"

"$(OUTDIR)\davinci.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\davinci.exe"

!ELSE 

ALL : "$(OUTDIR)\davinci.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\apifunc.obj"
	-@erase "$(INTDIR)\array.obj"
	-@erase "$(INTDIR)\dct.obj"
	-@erase "$(INTDIR)\error.obj"
	-@erase "$(INTDIR)\ff.obj"
	-@erase "$(INTDIR)\ff_ascii.obj"
	-@erase "$(INTDIR)\ff_avg.obj"
	-@erase "$(INTDIR)\ff_bbr.obj"
	-@erase "$(INTDIR)\ff_cluster.obj"
	-@erase "$(INTDIR)\ff_convolve.obj"
	-@erase "$(INTDIR)\ff_display.obj"
	-@erase "$(INTDIR)\ff_fft.obj"
	-@erase "$(INTDIR)\ff_gnoise.obj"
	-@erase "$(INTDIR)\ff_gplot.obj"
	-@erase "$(INTDIR)\ff_header.obj"
	-@erase "$(INTDIR)\ff_ifill.obj"
	-@erase "$(INTDIR)\ff_interp.obj"
	-@erase "$(INTDIR)\ff_ix.obj"
	-@erase "$(INTDIR)\ff_load.obj"
	-@erase "$(INTDIR)\ff_moment.obj"
	-@erase "$(INTDIR)\ff_pause.obj"
	-@erase "$(INTDIR)\ff_plplot.obj"
	-@erase "$(INTDIR)\ff_projection.obj"
	-@erase "$(INTDIR)\ff_random.obj"
	-@erase "$(INTDIR)\ff_rgb.obj"
	-@erase "$(INTDIR)\ff_sort.obj"
	-@erase "$(INTDIR)\ff_source.obj"
	-@erase "$(INTDIR)\ff_struct.obj"
	-@erase "$(INTDIR)\ff_text.obj"
	-@erase "$(INTDIR)\ff_version.obj"
	-@erase "$(INTDIR)\ff_vignette.obj"
	-@erase "$(INTDIR)\ff_write.obj"
	-@erase "$(INTDIR)\ff_xfrm.obj"
	-@erase "$(INTDIR)\fft.obj"
	-@erase "$(INTDIR)\fft2f.obj"
	-@erase "$(INTDIR)\fft_mayer.obj"
	-@erase "$(INTDIR)\fit.obj"
	-@erase "$(INTDIR)\help.obj"
	-@erase "$(INTDIR)\init.obj"
	-@erase "$(INTDIR)\io_ascii.obj"
	-@erase "$(INTDIR)\io_aviris.obj"
	-@erase "$(INTDIR)\io_byteswap.obj"
	-@erase "$(INTDIR)\io_ers.obj"
	-@erase "$(INTDIR)\io_goes.obj"
	-@erase "$(INTDIR)\io_grd.obj"
	-@erase "$(INTDIR)\io_imath.obj"
	-@erase "$(INTDIR)\io_isis.obj"
	-@erase "$(INTDIR)\io_lablib3.obj"
	-@erase "$(INTDIR)\io_magic.obj"
	-@erase "$(INTDIR)\io_pnm.obj"
	-@erase "$(INTDIR)\io_specpr.obj"
	-@erase "$(INTDIR)\io_themis.obj"
	-@erase "$(INTDIR)\io_vicar.obj"
	-@erase "$(INTDIR)\lexer.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\map.obj"
	-@erase "$(INTDIR)\matrix.obj"
	-@erase "$(INTDIR)\misc.obj"
	-@erase "$(INTDIR)\newfunc.obj"
	-@erase "$(INTDIR)\p.obj"
	-@erase "$(INTDIR)\parser.obj"
	-@erase "$(INTDIR)\pp.obj"
	-@erase "$(INTDIR)\pp_math.obj"
	-@erase "$(INTDIR)\printf.obj"
	-@erase "$(INTDIR)\reserved.obj"
	-@erase "$(INTDIR)\rfunc.obj"
	-@erase "$(INTDIR)\rpos.obj"
	-@erase "$(INTDIR)\scope.obj"
	-@erase "$(INTDIR)\string.obj"
	-@erase "$(INTDIR)\symbol.obj"
	-@erase "$(INTDIR)\system.obj"
	-@erase "$(INTDIR)\ufunc.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\vc50.pdb"
	-@erase "$(INTDIR)\w_readline.obj"
	-@erase "$(OUTDIR)\davinci.exe"
	-@erase "$(OUTDIR)\davinci.ilk"
	-@erase "$(OUTDIR)\davinci.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MLd /Gm /GX /Zi /Od /I "win32" /D "_DEBUG" /D\
 "HAVE_LIBREADLINE" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "__MSDOS__" /D\
 "LITTLE_E" /Fp"$(INTDIR)\davinci.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD\
 /c 
CPP_OBJS=.\Debug/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\davinci.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=hdf5.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)\davinci.pdb" /debug /machine:I386 /out:"$(OUTDIR)\davinci.exe"\
 /pdbtype:sept /libpath:"win32" 
LINK32_OBJS= \
	"$(INTDIR)\apifunc.obj" \
	"$(INTDIR)\array.obj" \
	"$(INTDIR)\dct.obj" \
	"$(INTDIR)\error.obj" \
	"$(INTDIR)\ff.obj" \
	"$(INTDIR)\ff_ascii.obj" \
	"$(INTDIR)\ff_avg.obj" \
	"$(INTDIR)\ff_bbr.obj" \
	"$(INTDIR)\ff_cluster.obj" \
	"$(INTDIR)\ff_convolve.obj" \
	"$(INTDIR)\ff_display.obj" \
	"$(INTDIR)\ff_fft.obj" \
	"$(INTDIR)\ff_gnoise.obj" \
	"$(INTDIR)\ff_gplot.obj" \
	"$(INTDIR)\ff_header.obj" \
	"$(INTDIR)\ff_ifill.obj" \
	"$(INTDIR)\ff_interp.obj" \
	"$(INTDIR)\ff_ix.obj" \
	"$(INTDIR)\ff_load.obj" \
	"$(INTDIR)\ff_moment.obj" \
	"$(INTDIR)\ff_pause.obj" \
	"$(INTDIR)\ff_plplot.obj" \
	"$(INTDIR)\ff_projection.obj" \
	"$(INTDIR)\ff_random.obj" \
	"$(INTDIR)\ff_rgb.obj" \
	"$(INTDIR)\ff_sort.obj" \
	"$(INTDIR)\ff_source.obj" \
	"$(INTDIR)\ff_struct.obj" \
	"$(INTDIR)\ff_text.obj" \
	"$(INTDIR)\ff_version.obj" \
	"$(INTDIR)\ff_vignette.obj" \
	"$(INTDIR)\ff_write.obj" \
	"$(INTDIR)\ff_xfrm.obj" \
	"$(INTDIR)\fft.obj" \
	"$(INTDIR)\fft2f.obj" \
	"$(INTDIR)\fft_mayer.obj" \
	"$(INTDIR)\fit.obj" \
	"$(INTDIR)\help.obj" \
	"$(INTDIR)\init.obj" \
	"$(INTDIR)\io_ascii.obj" \
	"$(INTDIR)\io_aviris.obj" \
	"$(INTDIR)\io_byteswap.obj" \
	"$(INTDIR)\io_ers.obj" \
	"$(INTDIR)\io_goes.obj" \
	"$(INTDIR)\io_grd.obj" \
	"$(INTDIR)\io_imath.obj" \
	"$(INTDIR)\io_isis.obj" \
	"$(INTDIR)\io_lablib3.obj" \
	"$(INTDIR)\io_magic.obj" \
	"$(INTDIR)\io_pnm.obj" \
	"$(INTDIR)\io_specpr.obj" \
	"$(INTDIR)\io_themis.obj" \
	"$(INTDIR)\io_vicar.obj" \
	"$(INTDIR)\lexer.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\map.obj" \
	"$(INTDIR)\matrix.obj" \
	"$(INTDIR)\misc.obj" \
	"$(INTDIR)\newfunc.obj" \
	"$(INTDIR)\p.obj" \
	"$(INTDIR)\parser.obj" \
	"$(INTDIR)\pp.obj" \
	"$(INTDIR)\pp_math.obj" \
	"$(INTDIR)\printf.obj" \
	"$(INTDIR)\reserved.obj" \
	"$(INTDIR)\rfunc.obj" \
	"$(INTDIR)\rpos.obj" \
	"$(INTDIR)\scope.obj" \
	"$(INTDIR)\string.obj" \
	"$(INTDIR)\symbol.obj" \
	"$(INTDIR)\system.obj" \
	"$(INTDIR)\ufunc.obj" \
	"$(INTDIR)\w_readline.obj"

"$(OUTDIR)\davinci.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "davinci - Win32 Release" || "$(CFG)" ==\
 "davinci - Win32 Debug"
SOURCE=.\apifunc.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_APIFU=\
	".\api.h"\
	".\apidef.h"\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\apifunc.obj" : $(SOURCE) $(DEP_CPP_APIFU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"


"$(INTDIR)\apifunc.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\array.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_ARRAY=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\array.obj" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_ARRAY=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\array.obj" : $(SOURCE) $(DEP_CPP_ARRAY) "$(INTDIR)"


!ENDIF 

SOURCE=.\dct.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_DCT_C=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\dct.obj" : $(SOURCE) $(DEP_CPP_DCT_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_DCT_C=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\dct.obj" : $(SOURCE) $(DEP_CPP_DCT_C) "$(INTDIR)"


!ENDIF 

SOURCE=.\error.c

"$(INTDIR)\error.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ff.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_C6=\
	".\apidef.h"\
	".\config.h"\
	".\ff.h"\
	".\func.h"\
	".\parser.h"\
	".\readline\history.h"\
	".\rfunc.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff.obj" : $(SOURCE) $(DEP_CPP_FF_C6) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_C6=\
	".\apidef.h"\
	".\ff.h"\
	".\func.h"\
	".\parser.h"\
	".\readline\history.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff.obj" : $(SOURCE) $(DEP_CPP_FF_C6) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_ascii.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_AS=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_ascii.obj" : $(SOURCE) $(DEP_CPP_FF_AS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_AS=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_ascii.obj" : $(SOURCE) $(DEP_CPP_FF_AS) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_avg.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_AV=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_avg.obj" : $(SOURCE) $(DEP_CPP_FF_AV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_AV=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_avg.obj" : $(SOURCE) $(DEP_CPP_FF_AV) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_bbr.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_BB=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_bbr.obj" : $(SOURCE) $(DEP_CPP_FF_BB) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_BB=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_bbr.obj" : $(SOURCE) $(DEP_CPP_FF_BB) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_cluster.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_CL=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_cluster.obj" : $(SOURCE) $(DEP_CPP_FF_CL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_CL=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_cluster.obj" : $(SOURCE) $(DEP_CPP_FF_CL) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_convolve.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_CO=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_convolve.obj" : $(SOURCE) $(DEP_CPP_FF_CO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_CO=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_convolve.obj" : $(SOURCE) $(DEP_CPP_FF_CO) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_display.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_DI=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_display.obj" : $(SOURCE) $(DEP_CPP_FF_DI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_DI=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_display.obj" : $(SOURCE) $(DEP_CPP_FF_DI) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_fft.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_FF=\
	".\config.h"\
	".\fft.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_fft.obj" : $(SOURCE) $(DEP_CPP_FF_FF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_FF=\
	".\fft.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_fft.obj" : $(SOURCE) $(DEP_CPP_FF_FF) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_gnoise.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_GN=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_gnoise.obj" : $(SOURCE) $(DEP_CPP_FF_GN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_GN=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_gnoise.obj" : $(SOURCE) $(DEP_CPP_FF_GN) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_gplot.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_GP=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_gplot.obj" : $(SOURCE) $(DEP_CPP_FF_GP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_GP=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_gplot.obj" : $(SOURCE) $(DEP_CPP_FF_GP) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_header.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_HE=\
	".\config.h"\
	".\func.h"\
	".\io_specpr.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_header.obj" : $(SOURCE) $(DEP_CPP_FF_HE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_HE=\
	".\func.h"\
	".\io_specpr.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_header.obj" : $(SOURCE) $(DEP_CPP_FF_HE) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_ifill.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_IF=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_ifill.obj" : $(SOURCE) $(DEP_CPP_FF_IF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_IF=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_ifill.obj" : $(SOURCE) $(DEP_CPP_FF_IF) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_interp.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_IN=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_interp.obj" : $(SOURCE) $(DEP_CPP_FF_IN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_IN=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_interp.obj" : $(SOURCE) $(DEP_CPP_FF_IN) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_ix.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_IX=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_ix.obj" : $(SOURCE) $(DEP_CPP_FF_IX) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_IX=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_ix.obj" : $(SOURCE) $(DEP_CPP_FF_IX) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_load.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_LO=\
	".\config.h"\
	".\func.h"\
	".\io_specpr.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_load.obj" : $(SOURCE) $(DEP_CPP_FF_LO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_LO=\
	".\func.h"\
	".\io_specpr.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_load.obj" : $(SOURCE) $(DEP_CPP_FF_LO) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_moment.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_MO=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_moment.obj" : $(SOURCE) $(DEP_CPP_FF_MO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_MO=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_moment.obj" : $(SOURCE) $(DEP_CPP_FF_MO) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_pause.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_PA=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_pause.obj" : $(SOURCE) $(DEP_CPP_FF_PA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_PA=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_pause.obj" : $(SOURCE) $(DEP_CPP_FF_PA) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_plplot.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_PL=\
	".\api_extern_defs.h"\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_plplot.obj" : $(SOURCE) $(DEP_CPP_FF_PL) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_PL=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_plplot.obj" : $(SOURCE) $(DEP_CPP_FF_PL) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_projection.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_PR=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	
NODEP_CPP_FF_PR=\
	".\projects.h"\
	

"$(INTDIR)\ff_projection.obj" : $(SOURCE) $(DEP_CPP_FF_PR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_PR=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_projection.obj" : $(SOURCE) $(DEP_CPP_FF_PR) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_random.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_RA=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_random.obj" : $(SOURCE) $(DEP_CPP_FF_RA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_RA=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_random.obj" : $(SOURCE) $(DEP_CPP_FF_RA) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_rgb.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_RG=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_rgb.obj" : $(SOURCE) $(DEP_CPP_FF_RG) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_RG=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_rgb.obj" : $(SOURCE) $(DEP_CPP_FF_RG) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_sort.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_SO=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_sort.obj" : $(SOURCE) $(DEP_CPP_FF_SO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_SO=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_sort.obj" : $(SOURCE) $(DEP_CPP_FF_SO) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_source.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_SOU=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_source.obj" : $(SOURCE) $(DEP_CPP_FF_SOU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_SOU=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_source.obj" : $(SOURCE) $(DEP_CPP_FF_SOU) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_struct.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_ST=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_struct.obj" : $(SOURCE) $(DEP_CPP_FF_ST) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_ST=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_struct.obj" : $(SOURCE) $(DEP_CPP_FF_ST) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_text.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_TE=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_text.obj" : $(SOURCE) $(DEP_CPP_FF_TE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_TE=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_text.obj" : $(SOURCE) $(DEP_CPP_FF_TE) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_version.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_VE=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\version.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_version.obj" : $(SOURCE) $(DEP_CPP_FF_VE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_VE=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\version.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_version.obj" : $(SOURCE) $(DEP_CPP_FF_VE) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_vignette.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_VI=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_vignette.obj" : $(SOURCE) $(DEP_CPP_FF_VI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_VI=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_vignette.obj" : $(SOURCE) $(DEP_CPP_FF_VI) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_write.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_WR=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_write.obj" : $(SOURCE) $(DEP_CPP_FF_WR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_WR=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_write.obj" : $(SOURCE) $(DEP_CPP_FF_WR) "$(INTDIR)"


!ENDIF 

SOURCE=.\ff_xfrm.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FF_XF=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ff_xfrm.obj" : $(SOURCE) $(DEP_CPP_FF_XF) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FF_XF=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ff_xfrm.obj" : $(SOURCE) $(DEP_CPP_FF_XF) "$(INTDIR)"


!ENDIF 

SOURCE=.\fft.c
DEP_CPP_FFT_C=\
	".\fft.h"\
	

"$(INTDIR)\fft.obj" : $(SOURCE) $(DEP_CPP_FFT_C) "$(INTDIR)"


SOURCE=.\fft2f.c

"$(INTDIR)\fft2f.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\fft_mayer.c
DEP_CPP_FFT_M=\
	".\trigtbl.h"\
	

"$(INTDIR)\fft_mayer.obj" : $(SOURCE) $(DEP_CPP_FFT_M) "$(INTDIR)"


SOURCE=.\fit.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_FIT_C=\
	".\config.h"\
	".\fit.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\fit.obj" : $(SOURCE) $(DEP_CPP_FIT_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_FIT_C=\
	".\fit.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\fit.obj" : $(SOURCE) $(DEP_CPP_FIT_C) "$(INTDIR)"


!ENDIF 

SOURCE=.\help.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_HELP_=\
	".\config.h"\
	".\func.h"\
	".\help.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\help.obj" : $(SOURCE) $(DEP_CPP_HELP_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_HELP_=\
	".\func.h"\
	".\help.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\help.obj" : $(SOURCE) $(DEP_CPP_HELP_) "$(INTDIR)"


!ENDIF 

SOURCE=.\init.c

"$(INTDIR)\init.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\io_ascii.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_IO_AS=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\io_ascii.obj" : $(SOURCE) $(DEP_CPP_IO_AS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_IO_AS=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\io_ascii.obj" : $(SOURCE) $(DEP_CPP_IO_AS) "$(INTDIR)"


!ENDIF 

SOURCE=.\io_aviris.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_IO_AV=\
	".\config.h"\
	".\func.h"\
	".\io_vicar.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\io_aviris.obj" : $(SOURCE) $(DEP_CPP_IO_AV) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_IO_AV=\
	".\func.h"\
	".\io_vicar.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\io_aviris.obj" : $(SOURCE) $(DEP_CPP_IO_AV) "$(INTDIR)"


!ENDIF 

SOURCE=.\io_byteswap.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_IO_BY=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\io_byteswap.obj" : $(SOURCE) $(DEP_CPP_IO_BY) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_IO_BY=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\io_byteswap.obj" : $(SOURCE) $(DEP_CPP_IO_BY) "$(INTDIR)"


!ENDIF 

SOURCE=.\io_ers.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_IO_ER=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\io_ers.obj" : $(SOURCE) $(DEP_CPP_IO_ER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_IO_ER=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\io_ers.obj" : $(SOURCE) $(DEP_CPP_IO_ER) "$(INTDIR)"


!ENDIF 

SOURCE=.\io_goes.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_IO_GO=\
	".\config.h"\
	".\func.h"\
	".\io_goes.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\io_goes.obj" : $(SOURCE) $(DEP_CPP_IO_GO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_IO_GO=\
	".\func.h"\
	".\io_goes.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\io_goes.obj" : $(SOURCE) $(DEP_CPP_IO_GO) "$(INTDIR)"


!ENDIF 

SOURCE=.\io_grd.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_IO_GR=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\io_grd.obj" : $(SOURCE) $(DEP_CPP_IO_GR) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_IO_GR=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\io_grd.obj" : $(SOURCE) $(DEP_CPP_IO_GR) "$(INTDIR)"


!ENDIF 

SOURCE=.\io_imath.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_IO_IM=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\io_imath.obj" : $(SOURCE) $(DEP_CPP_IO_IM) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_IO_IM=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\io_imath.obj" : $(SOURCE) $(DEP_CPP_IO_IM) "$(INTDIR)"


!ENDIF 

SOURCE=.\io_isis.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_IO_IS=\
	".\config.h"\
	".\func.h"\
	".\io_lablib3.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\toolbox.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\io_isis.obj" : $(SOURCE) $(DEP_CPP_IO_IS) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_IO_IS=\
	".\func.h"\
	".\io_lablib3.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\toolbox.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\io_isis.obj" : $(SOURCE) $(DEP_CPP_IO_IS) "$(INTDIR)"


!ENDIF 

SOURCE=.\io_lablib3.c
DEP_CPP_IO_LA=\
	".\io_lablib3.h"\
	".\toolbox.h"\
	

"$(INTDIR)\io_lablib3.obj" : $(SOURCE) $(DEP_CPP_IO_LA) "$(INTDIR)"


SOURCE=.\io_magic.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_IO_MA=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\io_magic.obj" : $(SOURCE) $(DEP_CPP_IO_MA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"


"$(INTDIR)\io_magic.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\io_pnm.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_IO_PN=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\io_pnm.obj" : $(SOURCE) $(DEP_CPP_IO_PN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_IO_PN=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\io_pnm.obj" : $(SOURCE) $(DEP_CPP_IO_PN) "$(INTDIR)"


!ENDIF 

SOURCE=.\io_specpr.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_IO_SP=\
	".\config.h"\
	".\func.h"\
	".\io_specpr.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\io_specpr.obj" : $(SOURCE) $(DEP_CPP_IO_SP) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_IO_SP=\
	".\func.h"\
	".\io_specpr.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\io_specpr.obj" : $(SOURCE) $(DEP_CPP_IO_SP) "$(INTDIR)"


!ENDIF 

SOURCE=.\io_themis.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_IO_TH=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\io_themis.obj" : $(SOURCE) $(DEP_CPP_IO_TH) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_IO_TH=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	

"$(INTDIR)\io_themis.obj" : $(SOURCE) $(DEP_CPP_IO_TH) "$(INTDIR)"


!ENDIF 

SOURCE=.\io_vicar.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_IO_VI=\
	".\config.h"\
	".\func.h"\
	".\io_vicar.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\io_vicar.obj" : $(SOURCE) $(DEP_CPP_IO_VI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_IO_VI=\
	".\func.h"\
	".\io_vicar.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\io_vicar.obj" : $(SOURCE) $(DEP_CPP_IO_VI) "$(INTDIR)"


!ENDIF 

SOURCE=.\lexer.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_LEXER=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	".\y_tab.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\lexer.obj" : $(SOURCE) $(DEP_CPP_LEXER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_LEXER=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\y_tab.h"\
	

"$(INTDIR)\lexer.obj" : $(SOURCE) $(DEP_CPP_LEXER) "$(INTDIR)"


!ENDIF 

SOURCE=.\main.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_MAIN_=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	".\y_tab.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_MAIN_=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\y_tab.h"\
	

"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"


!ENDIF 

SOURCE=.\map.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_MAP_C=\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\map.obj" : $(SOURCE) $(DEP_CPP_MAP_C) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"


"$(INTDIR)\map.obj" : $(SOURCE) "$(INTDIR)"


!ENDIF 

SOURCE=.\matrix.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_MATRI=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\matrix.obj" : $(SOURCE) $(DEP_CPP_MATRI) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_MATRI=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\matrix.obj" : $(SOURCE) $(DEP_CPP_MATRI) "$(INTDIR)"


!ENDIF 

SOURCE=.\misc.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_MISC_=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\misc.obj" : $(SOURCE) $(DEP_CPP_MISC_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_MISC_=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\misc.obj" : $(SOURCE) $(DEP_CPP_MISC_) "$(INTDIR)"


!ENDIF 

SOURCE=.\newfunc.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_NEWFU=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\newfunc.obj" : $(SOURCE) $(DEP_CPP_NEWFU) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_NEWFU=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\newfunc.obj" : $(SOURCE) $(DEP_CPP_NEWFU) "$(INTDIR)"


!ENDIF 

SOURCE=.\p.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_P_C70=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\p.obj" : $(SOURCE) $(DEP_CPP_P_C70) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_P_C70=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\p.obj" : $(SOURCE) $(DEP_CPP_P_C70) "$(INTDIR)"


!ENDIF 

SOURCE=.\parser.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_PARSE=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\parser.obj" : $(SOURCE) $(DEP_CPP_PARSE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_PARSE=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\parser.obj" : $(SOURCE) $(DEP_CPP_PARSE) "$(INTDIR)"


!ENDIF 

SOURCE=.\pp.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_PP_C74=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\pp.obj" : $(SOURCE) $(DEP_CPP_PP_C74) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_PP_C74=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\pp.obj" : $(SOURCE) $(DEP_CPP_PP_C74) "$(INTDIR)"


!ENDIF 

SOURCE=.\pp_math.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_PP_MA=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\pp_math.obj" : $(SOURCE) $(DEP_CPP_PP_MA) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_PP_MA=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\pp_math.obj" : $(SOURCE) $(DEP_CPP_PP_MA) "$(INTDIR)"


!ENDIF 

SOURCE=.\printf.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_PRINT=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\printf.obj" : $(SOURCE) $(DEP_CPP_PRINT) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_PRINT=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\printf.obj" : $(SOURCE) $(DEP_CPP_PRINT) "$(INTDIR)"


!ENDIF 

SOURCE=.\reserved.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_RESER=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\reserved.obj" : $(SOURCE) $(DEP_CPP_RESER) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_RESER=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\reserved.obj" : $(SOURCE) $(DEP_CPP_RESER) "$(INTDIR)"


!ENDIF 

SOURCE=.\rfunc.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_RFUNC=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\rfunc.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\rfunc.obj" : $(SOURCE) $(DEP_CPP_RFUNC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_RFUNC=\
	".\func.h"\
	".\parser.h"\
	".\rfunc.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\rfunc.obj" : $(SOURCE) $(DEP_CPP_RFUNC) "$(INTDIR)"


!ENDIF 

SOURCE=.\rpos.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_RPOS_=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\rpos.obj" : $(SOURCE) $(DEP_CPP_RPOS_) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_RPOS_=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\rpos.obj" : $(SOURCE) $(DEP_CPP_RPOS_) "$(INTDIR)"


!ENDIF 

SOURCE=.\scope.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_SCOPE=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\scope.obj" : $(SOURCE) $(DEP_CPP_SCOPE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_SCOPE=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\scope.obj" : $(SOURCE) $(DEP_CPP_SCOPE) "$(INTDIR)"


!ENDIF 

SOURCE=.\string.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_STRIN=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\string.obj" : $(SOURCE) $(DEP_CPP_STRIN) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_STRIN=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\string.obj" : $(SOURCE) $(DEP_CPP_STRIN) "$(INTDIR)"


!ENDIF 

SOURCE=.\symbol.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_SYMBO=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\symbol.obj" : $(SOURCE) $(DEP_CPP_SYMBO) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_SYMBO=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\symbol.obj" : $(SOURCE) $(DEP_CPP_SYMBO) "$(INTDIR)"


!ENDIF 

SOURCE=.\system.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_SYSTE=\
	".\config.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\system.obj" : $(SOURCE) $(DEP_CPP_SYSTE) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_SYSTE=\
	".\config.h"\
	

"$(INTDIR)\system.obj" : $(SOURCE) $(DEP_CPP_SYSTE) "$(INTDIR)"


!ENDIF 

SOURCE=.\ufunc.c

!IF  "$(CFG)" == "davinci - Win32 Release"

DEP_CPP_UFUNC=\
	".\config.h"\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	".\win32\values.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\ufunc.obj" : $(SOURCE) $(DEP_CPP_UFUNC) "$(INTDIR)"


!ELSEIF  "$(CFG)" == "davinci - Win32 Debug"

DEP_CPP_UFUNC=\
	".\func.h"\
	".\parser.h"\
	".\scope.h"\
	".\system.h"\
	".\ufunc.h"\
	".\values.h"\
	".\win32\_defs.h"\
	".\win32\H5ACpublic.h"\
	".\win32\H5api_adpt.h"\
	".\win32\H5Apublic.h"\
	".\win32\H5Bpublic.h"\
	".\win32\H5config.h"\
	".\win32\H5Dpublic.h"\
	".\win32\H5Epublic.h"\
	".\win32\H5Fpublic.h"\
	".\win32\H5Gpublic.h"\
	".\win32\H5HGpublic.h"\
	".\win32\H5HLpublic.h"\
	".\win32\H5Ipublic.h"\
	".\win32\H5MFpublic.h"\
	".\win32\H5MMpublic.h"\
	".\win32\H5Opublic.h"\
	".\win32\H5Ppublic.h"\
	".\win32\H5public.h"\
	".\win32\H5RApublic.h"\
	".\win32\H5Rpublic.h"\
	".\win32\H5Spublic.h"\
	".\win32\H5Tpublic.h"\
	".\win32\H5Zpublic.h"\
	".\win32\hdf5.h"\
	

"$(INTDIR)\ufunc.obj" : $(SOURCE) $(DEP_CPP_UFUNC) "$(INTDIR)"


!ENDIF 

SOURCE=.\w_readline.c

"$(INTDIR)\w_readline.obj" : $(SOURCE) "$(INTDIR)"



!ENDIF 

