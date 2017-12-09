/*
  apolloammxenable.h

  author: Henryk Richter <henryk.richter@gmx.net>

  purpose: Enable AMMX (per task)

*/
#ifndef _INC_APOLLOAMMXENABLE_H
#define _INC_APOLLOAMMXENABLE_H

#include "asminterface.h"

/* 
  return 0 = fail
         1 = AMMX ready for use
*/
ASM int Apollo_EnableAMMX( void );

/* 
  1 = AMMX is ready for use
  0 = AMMX unavailable
*/
extern unsigned char Apollo_AMMXon;
extern unsigned char Apollo_AMMX2on;

#endif /* _INC_APOLLOAMMXENABLE_H */
