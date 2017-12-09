/*
  blitapollo.h

  author: Henryk Richter <henryk.richter@gmx.net>

  purpose: AMMX optimized SDL blitter functions

*/
#ifndef _INC_BLITAPOLLO_H
#define _INC_BLITAPOLLO_H

#include "asminterface.h"

/*
  interface for all functions 
  input:   iptr    = input in any color format
           optr    = output in any color format
	   istride = line width (total, in bytes) of input
	   ostride = line width (total, in bytes) of output
	   w       = width (in pixels) to copy
	   h       = height (in pixels) to copy
*/
#define APOLLOBLIT(_name_) \
ASM void _name_( ASMR(a0) unsigned char *iptr   ASMREG(a0),\
                 ASMR(a1) unsigned char *optr   ASMREG(a1),\
                 ASMR(d2) unsigned int  istride ASMREG(d2),\
		 ASMR(d3) unsigned int  ostride ASMREG(d3),\
                 ASMR(d0) unsigned int  w       ASMREG(d0),\
		 ASMR(d1) unsigned int  h       ASMREG(d1)\
);

#define APOLLOBLITPROTO \
ASM void (*)(    ASMR(a0) unsigned char *iptr   ASMREG(a0),\
                 ASMR(a1) unsigned char *optr   ASMREG(a1),\
                 ASMR(d2) unsigned int  istride ASMREG(d2),\
		 ASMR(d3) unsigned int  ostride ASMREG(d3),\
                 ASMR(d0) unsigned int  w       ASMREG(d0),\
		 ASMR(d1) unsigned int  h       ASMREG(d1)\
)

/* generic: copy bytes, note: width must be multiplied with byte per pixel before calling for this one */
APOLLOBLIT( ApolloCopyRect );

/* target: RGB565 */
APOLLOBLIT( ApolloARGBtoRGB565  );
APOLLOBLIT( ApolloRGBtoRGB565   );
APOLLOBLIT( ApolloBGRtoRGB565   );
APOLLOBLIT( ApolloBGRAtoRGB565   );

/*APOLLOBLIT( ApolloBGRAtoRGB565LE );*/ /* rewrite this one! */

/* target: RGB24 */
APOLLOBLIT( ApolloARGBtoRGB24   );
APOLLOBLIT( ApolloBGRAtoRGB24   );
APOLLOBLIT( ApolloBGRtoRGB24    );

/* target: RGB32 */
APOLLOBLIT( ApolloARGBtoBGRA    );


#endif /* _INC_BLITAPOLLO_H */
