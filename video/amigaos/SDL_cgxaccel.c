/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997, 1998, 1999, 2000  Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include <SDL_config.h>
#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id: SDL_cgxaccel.c,v 1.2 2002/11/20 08:51:36 gabry Exp $";
#endif

//#include "SDL_error.h"
//#include "SDL_endian.h"
#include "../SDL_sysvideo.h"
#include "../SDL_blit.h"
#include "SDL_video.h"
#include "SDL_cgxvideo.h"

#ifdef AROS
#include <stdlib.h>
#endif
#define Bug 
static int CGX_HWAccelBlit(SDL_Surface *src, SDL_Rect *srcrect,
					SDL_Surface *dst, SDL_Rect *dstrect);

// These are needed to avoid register troubles with gcc -O2!

#if 1
/*
 * it was: defined(__SASC) || defined(__PPC__) || defined(MORPHOS)
 *
 * This is a workaround for an old gcc 2.7.x bug...
 */

#define BMKBRP(a,b,c,d,e,f,g,h,i,j) BltMaskBitMapRastPort(a,b,c,d,e,f,g,h,i,j)
#define	BBRP(a,b,c,d,e,f,g,h,i) BltBitMapRastPort(a,b,c,d,e,f,g,h,i)
#define BBB(a,b,c,d,e,f,g,h,i,j,k) BltBitMap(a,b,c,d,e,f,g,h,i,j,k)
#else
void BMKBRP(struct BitMap *a,long b, long c,struct RastPort *d,long e,long f,long g,long h,unsigned long i,APTR j)
{BltMaskBitMapRastPort(a,b,c,d,e,f,g,h,i,j);}

void BBRP(struct BitMap *a,long b, long c,struct RastPort *d,long e,long f,long g,long h,unsigned long i)
{BltBitMapRastPort(a,b,c,d,e,f,g,h,i);}

void BBB(struct BitMap *a,long b, long c,struct BitMap *d,long e,long f,long g,long h,unsigned long i,unsigned long j,PLANEPTR k)
{BltBitMap(a,b,c,d,e,f,g,h,i,j,k);}
#endif

int CGX_SetHWColorKey(_THIS,SDL_Surface *surface, Uint32 key)
{
	return -1; // do no accel blits
	if(surface->hwdata)
	{
		if(surface->hwdata->mask)
			free(surface->hwdata->mask);

		if(surface->hwdata->mask=malloc(RASSIZE(surface->w,surface->h)))
		{
			Uint32 pitch,ok=0;
			APTR lock;

			memset(surface->hwdata->mask,255,RASSIZE(surface->w,surface->h));

			D(bug("Building colorkey mask: color: %ld, size: %ld x %ld, %ld bytes...Bpp:%ld\n",key,surface->w,surface->h,RASSIZE(surface->w,surface->h),surface->format->BytesPerPixel));

			if(lock=LockBitMapTags(surface->hwdata->bmap,LBMI_BASEADDRESS,(ULONG)&surface->pixels,
					LBMI_BYTESPERROW,(ULONG)&pitch,TAG_DONE))
			{
				switch(surface->format->BytesPerPixel)
				{
					case 1:
					{
						unsigned char k=key;
						register int i,j,t;
						register unsigned char *dest=surface->hwdata->mask,*map=surface->pixels;

						pitch-=surface->w;

						for(i=0;i<surface->h;i++)
						{
							for(t=128,j=0;j<surface->w;j++)
							{
								if(*map==k)
									*dest&=~t;	

								t>>=1;

								if(t==0)
								{
									dest++;
									t=128;
								}
								map++;
							}
							map+=pitch;
						}
					}
					break;
					case 2:
					{
						Uint16 k=key,*mapw;
						register int i,j,t;
						register unsigned char *dest=surface->hwdata->mask,*map=surface->pixels;

						for(i=surface->h;i;--i)
						{
							mapw=(Uint16 *)map;

							for(t=128,j=surface->w;j;--j)
							{
								if(*mapw==k)
									*dest&=~t;

								t>>=1;

								if(t==0)
								{
									dest++;
									t=128;
								}
								mapw++;
							}
							map+=pitch;
						}
					}
					break;
					case 4:
					{
						Uint32 *mapl;
						register int i,j,t;
						register unsigned char *dest=surface->hwdata->mask,*map=surface->pixels;

						for(i=surface->h;i;--i)
						{
							mapl=(Uint32 *)map;

							for(t=128,j=surface->w;j;--j)
							{
								if(*mapl==key)
									*dest&=~t;

								t>>=1;

								if(t==0)
								{
									dest++;
									t=128;
								}
								mapl++;
							}
							map+=pitch;
						}

					}
					break;
					default:
						D(bug("Pixel mode non supported for color key..."));
						free(surface->hwdata->mask);
						surface->hwdata->mask=NULL;
						ok=-1;
				}
				UnLockBitMap(lock);
				D(bug("...Colorkey built!\n"));
				return ok;
			}
		}
	}
	D(bug("HW colorkey not supported for this depth\n"));

	return -1;
}

int CGX_CheckHWBlit(_THIS,SDL_Surface *src,SDL_Surface *dst)
{
// Doesn't support yet alpha blitting
	//kprintf("HW blit\n");
	if (this->hidden->swap_bytes)return 0;
	if(src->hwdata&& !(src->flags & (SDL_SRCALPHA)))
	{
		D(bug("CheckHW blit... OK!\n"));

		if ( (src->flags & SDL_SRCCOLORKEY) == SDL_SRCCOLORKEY ) {
			if ( CGX_SetHWColorKey(this, src, src->format->colorkey) < 0 ) {
				src->flags &= ~SDL_HWACCEL;
				
				return 0;
			}
		}

		src->flags|=SDL_HWACCEL;
		src->map->hw_blit = CGX_HWAccelBlit;
		
		return 1;
	}
	else
		src->flags &= ~SDL_HWACCEL;

	D(bug("CheckHW blit... NO!\n"));

	return 0;
}

static int temprp_init=0;
static struct RastPort temprp;

static int CGX_HWAccelBlit(SDL_Surface *src, SDL_Rect *srcrect,
					SDL_Surface *dst, SDL_Rect *dstrect)
{
	struct SDL_VideoDevice *this=src->hwdata->videodata;
//	D(bug("Accel blit!\n"));
    //kprintf("Accel blit\n");
	if(src->flags&SDL_SRCCOLORKEY && src->hwdata->mask)
	{
		if(dst==SDL_VideoSurface)
		{
			BMKBRP(src->hwdata->bmap,srcrect->x,srcrect->y,
						SDL_RastPort,dstrect->x+SDL_Window->BorderLeft,dstrect->y+SDL_Window->BorderTop,
						srcrect->w,srcrect->h,0xc0,src->hwdata->mask);
		} 
		else if(dst->hwdata)
		{
			if(!temprp_init)
			{ 
				InitRastPort(&temprp);
				temprp_init=1;
			}
			temprp.BitMap=(struct BitMap *)dst->hwdata->bmap;
            if (temprp.BitMap)
			BMKBRP(src->hwdata->bmap,srcrect->x,srcrect->y,
						&temprp,dstrect->x,dstrect->y,
						srcrect->w,srcrect->h,0xc0,src->hwdata->mask);
			else
			BMKBRP(src->hwdata->bmap,srcrect->x,srcrect->y,
						SDL_RastPort,dstrect->x,dstrect->y,
						srcrect->w,srcrect->h,0xc0,src->hwdata->mask);
			
		}
	}
	
	/*if(dst==SDL_VideoSurface)
	{
		long b_src;
		if (!src->hwdata->bmap)b_src = SDL_RastPort->BitMap;
		else b_src = src->hwdata->bmap;
		BBRP(b_src,srcrect->x,srcrect->y,SDL_RastPort,dstrect->x+SDL_Window->BorderLeft,dstrect->y+SDL_Window->BorderTop,srcrect->w,srcrect->h,0xc0);
	}*/
	else if(dst->hwdata->bmap)
		BBB(src->hwdata->bmap,srcrect->x,srcrect->y,dst->hwdata->bmap,dstrect->x,dstrect->y,srcrect->w,srcrect->h,0xc0,0xff,NULL);
	else if(!dst->hwdata->bmap)
		BBB(src->hwdata->bmap,srcrect->x,srcrect->y,SDL_RastPort->BitMap,dstrect->x,dstrect->y,srcrect->w,srcrect->h,0xc0,0xff,NULL);
    
	return 0;
}

int CGX_FillHWRect(_THIS,SDL_Surface *dst,SDL_Rect *dstrect,Uint32 color)
{
	unsigned int handle;

	if (dst->format->Bmask == 0xff000000)color = SDL_Swap32(color); // for bgra32 mode data must swap.
	if (dst->format->Bmask == 0x1f)
	{
		Uint32 r,g,b;
		b = (color & 0x1f) << 3;  // from rgb16 mode data must change to fit the argb FillPixelArray format. 
		g = ((color >> 5) & 0x3f) << 2;
		r = ((color >> 11) & 0x1f) << 3;
        color = b | (g << 8) | (r << 16);
	}
	if(dst->hwdata)
	{
		if(!temprp_init)
		{
			InitRastPort(&temprp);
			temprp_init=1;
		}
        //kprintf("color %lx \n",color);
		
		temprp.BitMap=(struct BitMap *)dst->hwdata->bmap;
		if (temprp.BitMap == 0)temprp.BitMap = SDL_RastPort->BitMap;
		if (dst->format->BitsPerPixel == 8)  //because CGX fillpixelarray dont work on 8 bit screens
		{ 
			if (this->screen->hwdata->lock)
			{
					UnLockBitMap(this->screen->hwdata->lock);
		            SetAPen(&temprp,color);
			        RectFill(&temprp,dstrect->x,dstrect->y,dstrect->w + dstrect->x ,dstrect->h + dstrect->y);
					this->screen->hwdata->lock=LockBitMapTags(temprp.BitMap,LBMI_BASEADDRESS,(ULONG)&this->screen->pixels,
					TAG_DONE);
		    return ;
			}
			SetAPen(&temprp,color);
			        RectFill(&temprp,dstrect->x,dstrect->y,dstrect->w + dstrect->x ,dstrect->h + dstrect->y);
		}
		else
		FillPixelArray(&temprp,dstrect->x,dstrect->y,dstrect->w,dstrect->h,color);
	}
	return 0;
}
