SHELL = /bin/sh

CC = gcc
AR = ar
RANLIB = ranlib

CPPFLAGS = 
CFLAGS = $(CPPFLAGS) -g -O2 -I.  -I/usr/X11R6/include

VICARLIB = vicar.a

.c.o:
	$(CC) -c $(CFLAGS) $<

OBJS = XvicBasicImage.o XvicCopyRaw.o XvicImage.o XvicImageOverlay.o XvicRegion.o XvicStringCursor.o

all: $(OBJS)
	$(AR) ruv $(VICARLIB) $(OBJS)
	$(RANLIB) $(VICARLIB)

clean:
	rm -f $(OBJS) $(VICARLIB)
