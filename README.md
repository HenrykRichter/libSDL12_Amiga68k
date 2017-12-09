Disclaimer
==========

The Simple DirectMedia Layer (SDL for short) is a cross-platfrom library
designed to make it easy to write multi-media software, such as games and
emulators.

The Simple DirectMedia Layer library source code is available from:
http://www.libsdl.org/

This library is distributed under the terms of the GNU LGPL license:
http://www.gnu.org/copyleft/lesser.html


Introduction
------------

This is one of the ports of SDL to the AmigaOS3/68k platform. I cannot
comment much on the origins of the code contributions to this port. I've
kept the files I got into my hands, along with the various existing Makefiles.

The main purpose of this branch (based on SDL 1.2.14, AFAIK) was to provide 
some bugfixes and performance improvements. Towards the latter point, most of
the blitting functions have been rewritten in 68k ASM. Also, some of the routines
can switch to AMMX usage (where applicable). 

This SDL version should work on all Amigas with RTG (Picasso96 or CyberGraphX).


Building
--------

Several Makefiles for different targets are present in this codebase. The one
I was using for the AMMX enabled builds is "Makefile.bax". The compilers I've 
successfully used were gcc2.95 and gcc6.3.1b (20171120).

The static libSDL.a can be built by

    make -f Makefile.bax

I assume a cross-compile environment at /opt/amigaos-68k. Change the prefix path
as necessary. Compiler, Assembler and Linker binary names might need adjusting, too.
Please also have a look at VFLAGS. The old cross-compile toolchain I had was putting
the AmigaOS includes into "os-include". The recently released GCC6 toolchain by Bebbo
refers to "sys-include" instead.

My regular AMMX build is for libnix. If ixemul or clib2 is desired, then please refer to 
the other Makefiles to mix up an appropriate environment.


Usage
-----

When compiling SDL programs, I usually set the following flags to gcc: 

    -noixemul -O3 -fomit-frame-pointer -m68020-60 -msoft-float 

The Linker commands should include -noixemul and -lSDL

