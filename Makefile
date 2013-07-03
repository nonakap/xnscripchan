#	$Id: Makefile,v 1.16 2002/12/05 17:53:43 nonaka Exp $	

PROG= xnscripchan

#FONTSET= -DFONTSET="-ricoh-gothic-%s-r-normal--%ld-*-*-*-*-*-*-*"
#IMAGE_CACHE= -DIMAGE_CACHE=0

LEX= flex
YFLAGS= -d

COPTS= -g -O2 #-DSNAP -DDEBUG
CWARNS= -W -Wall -Wuninitialized -Wpointer-arith
CWARNS+= -Wmissing-prototypes -Wstrict-prototypes
CINCLUDES= -I/usr/pkg/include -I/usr/local/include -I/usr/X11R6/include
CFLAGS= $(COPTS) $(CWARNS) $(CINCLUDES) $(FONTSET)

LDFLAGS=
LIBS=
LIBS+= -L/usr/pkg/lib -R/usr/pkg/lib -L/usr/local/lib -R/usr/local/lib
LIBS+= -ljpeg -lpng -lbz2 -lfl
LIBS+= -L/usr/X11R6/lib -R/usr/X11R6/lib -lX11

CFLAGS+= -DUSE_LIBUNGIF
LIBS+= -lungif

OBJS= parse.o lex.o \
	_parse.o \
	archive.o nsa.o sar.o \
	button.o command.o label.o prescan.o select.o \
	misc.o stack.o symbol.o var.o timer.o \
	display.o screen.o \
	image.o effect.o sprite.o \
	jpeg.o bmp.o png.o spb.o gif.o \
	bitop.o lzss.o nbz.o \
	x11.o xdimage.o \
	main.o

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	-/bin/rm -f $(PROG) *.o y.tab.[ch] lex.yy.c core.* *.core

cleandir distclean: clean
