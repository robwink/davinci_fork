# Generated automatically from Makefile.in by configure.
SHELL = /bin/sh

srcdir = .

prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
datadir = $(prefix)/lib
mandir = $(prefix)/man/man1

INSTALL = .././install-sh -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644

CC     = gcc
DEFS   = -I/usr/openwin/include -DHAVE_CONFIG_H 
LIBS   = -L/usr/openwin/lib -lplplotFX -lMagick -ltiff -lproj -lreadline -ltermcap -ljpeg -lmsss_vis -lusds -lhdf5 -lz -lXm -lXext -lXt -lX11 -lm  -L/opt/local/src/ImageMagick-4.2.9/magick -lMagick -ltiff -ljpeg -lpng -ldpstk -ldps -lXext -lXt -lX11 -lsocket -lnsl -lz -lm -ldl

.c.o:
	$(CC) -c $(CPPFLAGS) $(DEFS) $(CFLAGS) $<

OBJ=Button.o Slider.o VisInfo.o Joystick.o MapAndWait.o init.o AMap.o \
    List.o xgets.o chd.o toggle.o confirm.o hershey.o Composite.o \
    LB.o MB.o PB.o RB.o CB.o XB.o xf.o misc.o 3D.o

libXfred.a:	$(OBJ) Xfred.h
	@echo "*** Making Xfred library ***"
	-rm -f libXfred.a
	ar cqv libXfred.a $(OBJ)
	-ranlib libXfred.a

clean:
	rm -f $(OBJ) libXfred.a

############################################################################
# DEPENDANCIES
############################################################################
Button.o:	Button.c Xfred.h Button.h Callback.h VisInfo.h
Slider.o:	Slider.c Xfred.h Slider.h VisInfo.h
VisInfo.o:	VisInfo.c VisInfo.h
Joystick.o:	Joystick.c Xfred.h Callback.h VisInfo.h Joystick.h
MapAndWait.o:	MapAndWait.c
init.o:		init.c
AMap.o:		AMap.c Xfred.h AMap.h Callback.h VisInfo.h
List.o: 	Button.c Slider.c Button.h Slider.h List.h VisInfo.h Callback.h
hershey.o:	hershey.c hershey.h
