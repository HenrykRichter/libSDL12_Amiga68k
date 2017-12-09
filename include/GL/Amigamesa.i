* Amigamesa.i

 *
 * Mesa 3-D graphics library
 * Version:  1.2
 * Copyright (C) 1995  Brian Paul  (brianp@ssec.wisc.edu)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *


* Important note (03.01.1998 by Sam Jordan)

* When using windows you should use window->BorderBottom for AMA_Bottom,
* not window->BorderBottom+1! Please change your existing MESA application
* when using this implementation here. The reason for this change is that
* the current implementation doesn't decrease the viewport size by 2 to
* get an additional border (which doesn't look nice and which didn't work
* correctly either).

* When using the TK window layer you don't have to change anything.


                IFND    AMIGAMESA_I
AMIGAMESA_I     SET     1


                IFND    EXEC_TYPES_I
                include exec/types.i
                ENDC

                IFND    EXEC_PORTS_I
                include exec/ports.i
                ENDC

                IFND    UTILITY_TAGITEM_I
                include utility/tagitem.i
                ENDC

* values of the GL include:

GL_DRIVERCTX    =       $a00            ;always check when upgrading MESA!


                STRUCTURE       AMIGAMESA_VISUAL,0
                APTR            AMV_VISUAL
                UBYTE           AMV_DB_FLAG
                UBYTE           AMV_RGB_FLAG
                UBYTE           AMV_ALPHA_FLAG
                UBYTE           AMV_PAD
                LONG            AMV_DEPTH
                LABEL           AMV_SIZE

                STRUCTURE       AMIGAMESA_BUFFER,0
                APTR            AMB_BUFFER
                LABEL           AMB_SIZE

                STRUCTURE       AMIGAMESA_CONTEXT,0
                APTR            AMC_GL_CTX
                APTR            AMC_VISUAL
                APTR            AMC_BUFFER
                APTR            AMC_SHARE
                ULONG           AMC_FLAGS
                ULONG           AMC_PIXEL
                ULONG           AMC_CLEARPIXEL
                APTR            AMC_WINDOW
                APTR            AMC_FRONT_RP
                APTR            AMC_BACK_RP
                LONG            AMC_SWAPCOUNTER
                APTR            AMC_FRONTARRAY
                APTR            AMC_BACKARRAY
                APTR            AMC_RP
                APTR            AMC_SCREEN
                APTR            AMC_TMPRAS
                APTR            AMC_TEMPRP
                APTR            AMC_DBUFINFO
                LONG            AMC_DEPTH
                LONG            AMC_BPPIX
                LONG            AMC_BPROW
                LONG            AMC_FMT
                LONG            AMC_WIDTH
                LONG            AMC_HEIGHT
                LONG            AMC_LEFT
                LONG            AMC_TOP
                LONG            AMC_RIGHT
                LONG            AMC_BOTTOM
                LONG            AMC_REALWIDTH
                LONG            AMC_REALHEIGHT
                LONG            AMC_FIXEDWIDTH
                LONG            AMC_FIXEDHEIGHT
                APTR            AMC_COLORTABLE
                APTR            AMC_COLORTABLE2
                STRUCT          AMC_PENCONV,256
                STRUCT          AMC_PENCONVINV,256
                STRUCT          AMC_DTABLE,256
                STRUCT          AMC_BUGFIX,256
                APTR            AMC_IMAGELINE
                APTR            AMC_RGB_BUFFER
                APTR            AMC_INITDD
                APTR            AMC_DISPOSE
                APTR            AMC_SWAPBUFFER
                STRUCT          AMC_COLTABLE,256*4
                STRUCT          AMC_COLTABLE2,256*4
                ULONG           AMC_OLDFPU
                STRUCT          AMC_PIXELARGB,4
                ULONG           AMC_SPECIALALLOC
                STRUCT          AMC_DMATRIX,128+8
                APTR            AMC_SBUF1
                APTR            AMC_SBUF2
                APTR            AMC_DBPORT
                LONG            AMC_DRAWBUFFERFLAG
                LONG            AMC_READBUFFERFLAG
                LONG            AMC_BACKARRAYFLAG
                APTR            AMC_DRAWBUFFER
                APTR            AMC_READBUFFER
                APTR            AMC_BACK2_RP
                APTR            AMC_DRAW_RP
                APTR            AMC_HWDRIVER
                APTR            AMC_SBUF3
                APTR            AMC_SBUF_INITIAL
                LABEL           AMC_SIZE

FLAG_OWNVISUAL          SET     1
FLAG_OWNBUFFER          SET     2
FLAG_FORBI3DHW          SET     4
FLAG_FULLSCREEN         SET     8
FLAG_DIRECTRENDER       SET     16
FLAG_TWOBUFFERS         SET     32
FLAG_FAST               SET     64
FLAG_VERYFAST           SET     128
FLAG_RGBA               SET     256
FLAG_SYNC               SET     512
FLAG_TRIPLE             SET     1024

AMA_DUMMY       SET     TAG_USER + 32
AMA_LEFT        SET     AMA_DUMMY + 1
AMA_BOTTOM      SET     AMA_DUMMY + 2
AMA_WIDTH       SET     AMA_DUMMY + 3
AMA_HEIGHT      SET     AMA_DUMMY + 4
AMA_DRAWMODE    SET     AMA_DUMMY + 5
AMA_SCREEN      SET     AMA_DUMMY + 6
AMA_WINDOW      SET     AMA_DUMMY + 7
AMA_RASTPORT    SET     AMA_DUMMY + 8
AMA_DOUBLEBUF   SET     AMA_DUMMY + $30
AMA_RGBMODE     SET     AMA_DUMMY + $31
AMA_ALPHAFLAG   SET     AMA_DUMMY + $32
AMA_FORBID3DHW  SET     AMA_DUMMY + $33
AMA_FULLSCREEN  SET     AMA_DUMMY + $34
AMA_DIRECTRENDER SET   AMA_DUMMY + $35
AMA_TWOBUFFERS  SET   AMA_DUMMY + $36
AMA_FAST        SET     AMA_DUMMY + $37
AMA_VERYFAST    SET     AMA_DUMMY + $38
AMA_NODEPTH     SET     AMA_DUMMY + $39
AMA_NOSTENCIL   SET     AMA_DUMMY + $3a
AMA_NOACCUM     SET     AMA_DUMMY + $3b
AMA_VISUAL      SET     AMA_DUMMY + $41
AMA_BUFFER      SET     AMA_DUMMY + $42
AMA_WINDOWID    SET     AMA_DUMMY + $43
                ENDC
