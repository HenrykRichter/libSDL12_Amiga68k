/*
  colorkeyapollo.h

  author: Henryk Richter <henryk.richter@gmx.net>

  purpose: AMMX optimized SDL blitter functions

*/
#ifndef _INC_COLORKEYAPOLLO_H
#define _INC_COLORKEYAPOLLO_H

#include "asminterface.h"

/*
  interface for all functions 
  input:   iptr    = input in any color format
           optr    = output in any color format
	   istride = line width (total, in bytes) of input
	   ostride = line width (total, in bytes) of output
	   w       = width (in pixels) to copy
	   h       = height (in pixels) to copy
	   key     = color key to compare (if match, no write)
*/
#define APOLLOCBLIT(_name_) \
ASM void _name_( ASMR(a0) unsigned char *iptr   ASMREG(a0),\
                 ASMR(a1) unsigned char *optr   ASMREG(a1),\
                 ASMR(d2) unsigned int  istride ASMREG(d2),\
		 ASMR(d3) unsigned int  ostride ASMREG(d3),\
                 ASMR(d0) unsigned int  w       ASMREG(d0),\
		 ASMR(d1) unsigned int  h       ASMREG(d1),\
		 ASMR(d6) unsigned int  key     ASMREG(d6)\
);


/* target: RGB565 */
//APOLLOCBLIT( ApolloARGBtoRGB565  );
//APOLLOCBLIT( ApolloRGBtoRGB565   );
//APOLLOCBLIT( ApolloBGRtoRGB565   );
//APOLLOCBLIT( ApolloBGRAtoRGB565   );

/* target: RGB24 */
//APOLLOCBLIT( ApolloARGBtoRGB24   );
//APOLLOCBLIT( ApolloBGRAtoRGB24   );
//APOLLOCBLIT( ApolloBGRtoRGB24    );

/* target: RGB32 */
//APOLLOCBLIT( ApolloKeyARGBtoARGB   );

APOLLOCBLIT( ApolloKeyRGB565toRGB565 );


#endif /* _INC_COLORKEYAPOLLO_H */
