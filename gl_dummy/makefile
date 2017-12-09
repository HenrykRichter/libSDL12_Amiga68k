DEFINES=  DEFINE=inline=__inline 
CFLAGS = VERBOSE CPU=68040 CODE=FAR DATA=NEAR
GCCFLAGS =  -DFULL_SPECIFIERS -m68040 -I../headers/ -O3 -fomit-frame-pointer  -mnobitfield
GLFLAGS = -DSHARED_LIB 

#GCCDEFINES = -DNO_SIGNAL_H

OBJS = dummy.o
   
%.o: %.c
	$(CC) $(GCCFLAGS) $(GCCDEFINES) -o $@ -c $*.c


#%.wo: %.c
#	ppc-amigaos-gcc -warpup $(GCCFLAGS) $(GCCDEFINES) -DWARPOS -DNO_AMIGADEBUG -o $@ -c $*.c

all: libgl_dummy.a 

# gg:lib/libb/libnix/libnix.a gg:lib/libb/libnix/libstubs.a  gg:lib/libb/libnix/libm.a -o $@ -ldebug

#	slink to $@ LIBFD SDL.fd lib lib:sc.lib lib:scm.lib lib:debug.lib noicons libversion 1 librevision 2 smalldata stripdebug libprefix _LIB from lib:libent.o lib:libinitr.o $(OBJS) SDL_amigasas.so
#	gcc -v -fbaserel -nostartfiles -noixemul gg:lib/libnix/libinitr.o $(LOBJS) -o $@ -lm -ldebug
#	gcc -v -s -resident -nostdlib gg:lib/libnix/libinitr.o $(LOBJS) -o $@ -ldebug

libgl_dummy.a: $(OBJS)
	-rm -f libgl_dummy.a
	ar cru libgl_dummy.a $(OBJS)
	/usr/local/amiga/bin/ranlib libgl_dummy.a
	-cp libgl_dummy.a /usr/local/amiga/m68k-amigaos/lib/

clean: 
	-rm -f $(OBJS) 


 
