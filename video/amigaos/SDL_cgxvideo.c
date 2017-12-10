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

#ifdef SAVE_RCSID
static char rcsid =
 "@(#) $Id: SDL_cgxvideo.c,v 1.2.3 2008/11/20 08:51:50 bernd roesch Exp $";
#endif

/*
 * CGX based SDL video driver implementation by Gabriele Greco
 * gabriele.greco@aruba.it
 */


#include "SDL_config.h" 
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#ifdef MORPHOS
#include <cybergraphx/cybergraphics.h>
#else
#include <cybergraphics/cybergraphics.h>
#endif

#include "SDL_video.h"
#include "SDL_mouse.h"
//#include "SDL_endian.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"
#include "SDL_cgxgl_c.h"
#include "SDL_cgxvideo.h"
#include "SDL_cgxwm_c.h"
#include "SDL_amigamouse_c.h"
#include "SDL_amigaevents_c.h"
#include "SDL_cgxmodes_c.h"
#include "SDL_cgximage_c.h"
#include "SDL_cgxyuv_c.h"
 
#ifdef APOLLO_BLIT
#include "apolloammxenable.h"
#endif

#ifdef _AROS
#include <stdlib.h>
#include <proto/alib.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif
//#define Bug kprintf

unsigned long _sdl_windowaddr;

/* Initialization/Query functions */
static int CGX_VideoInit(_THIS, SDL_PixelFormat *vformat);
static SDL_Surface *CGX_SetVideoMode(_THIS, SDL_Surface *current, int width, int height, int bpp, Uint32 flags);
static int CGX_ToggleFullScreen(_THIS, int on);
static void CGX_UpdateMouse(_THIS);
static int CGX_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors);
static void CGX_VideoQuit(_THIS);
static struct SignalSemaphore sem;
/* CGX driver bootstrap functions */

struct Library *CyberGfxBase=NULL;
struct IntuitionBase *IntuitionBase=NULL;
struct GfxBase *GfxBase=NULL;

void SDL_AmigaLockWindow()
{
ObtainSemaphore (&sem);
}

void SDL_AmigaUnlockWindow()
{
ReleaseSemaphore (&sem);
}

struct Window * SDL_AmigaWindowAddr(void)
{
  return (struct Window *)_sdl_windowaddr;
}

int CGX_SetGamma(_THIS, float red, float green, float blue)
{
    SDL_SetError("Gamma correction not supported");
    return -1;
}

int CGX_GetGamma(_THIS, float red, float green, float blue)
{
    SDL_SetError("Gamma correction not supported");
    return -1;
}

int CGX_SetGammaRamp(_THIS, Uint16 *ramp)
{
#if 0
	Int i, ncolors;
	XColor xcmap[256];

	/* See if actually setting the gamma is supported */
	if ( SDL_Visual->class != DirectColor ) {
	    SDL_SetError("Gamma correction not supported on this visual");
	    return(-1);
	}

	/* Calculate the appropriate palette for the given gamma ramp */
	ncolors = SDL_Visual->map_entries;
	for ( i=0; i<ncolors; ++i ) {
		Uint8 c = (256 * i / ncolors);
		xcmap[i].pixel = SDL_MapRGB(this->screen->format, c, c, c);
		xcmap[i].red   = ramp[0*256+c];
		xcmap[i].green = ramp[1*256+c];
		xcmap[i].blue  = ramp[2*256+c];
		xcmap[i].flags = (DoRed|DoGreen|DoBlue);
	}
	XStoreColors(GFX_Display, SDL_XColorMap, xcmap, ncolors);
	XSync(GFX_Display, False);

	return(0);

#else
    SDL_SetError("Gamma correction not supported on this visual");
    return(-1);

#endif
}

static void DestroyScreen(_THIS)
{
  	if(currently_fullscreen)
	{
		if(this->hidden->dbuffer == 2)
		{
			this->hidden->dbuffer=0;

			if(SDL_RastPort && SDL_RastPort != &SDL_Display->RastPort)
				free(SDL_RastPort);
			SDL_RastPort=NULL;
		}
		if(this->hidden->dbuffer == 1)
		{
			extern struct MsgPort *safeport,*dispport;

			this->hidden->dbuffer=0;

			if(safeport)
			{
				while(GetMsg(safeport)!=NULL);
				DeleteMsgPort(safeport);
			}
			if(dispport)
			{
				while(GetMsg(dispport)!=NULL);
				DeleteMsgPort(dispport);
			}

			this->hidden->SB[0]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort=this->hidden->SB[0]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort=NULL;
			this->hidden->SB[1]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort=this->hidden->SB[1]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort=NULL;

			if(this->hidden->SB[1])
				FreeScreenBuffer(SDL_Display,this->hidden->SB[1]);
			if(this->hidden->SB[0])
				FreeScreenBuffer(SDL_Display,this->hidden->SB[0]);


			this->hidden->SB[0]=this->hidden->SB[1]=NULL;

			if(SDL_RastPort && SDL_RastPort != &SDL_Display->RastPort)
				free(SDL_RastPort);

			SDL_RastPort=NULL;
		}
		CloseScreen(GFX_Display);
		currently_fullscreen=0;
	}
	else if(GFX_Display)
		UnlockPubScreen(NULL,GFX_Display);

	GFX_Display = NULL;
}

static int CGX_Available(void)
{
	struct Library *l;

	l = OpenLibrary("cybergraphics.library",0L);

	if ( l != NULL ) {
		D(bug("CGX video device AVAILABLE\n"));
		CloseLibrary(l);
	}
	D(else bug("**CGX video device UNAVAILABLE\n"));

	return(l != NULL);
}

static void CGX_DeleteDevice(SDL_VideoDevice *device)
{
	if ( device ) {
		if ( device->hidden ) {
			free(device->hidden);
		}
		if ( device->gl_data ) {
			free(device->gl_data);
		}
		free(device);
	}
}

static SDL_VideoDevice *CGX_CreateDevice(int devindex)
{
	SDL_VideoDevice *device;
    InitSemaphore(&sem);
	/* Initialize all variables that we clean on shutdown */
	device = (SDL_VideoDevice *)malloc(sizeof(SDL_VideoDevice));
	if ( device ) {
		memset(device, 0, (sizeof *device));
		device->hidden = (struct SDL_PrivateVideoData *)
				malloc((sizeof *device->hidden));
		device->gl_data = (struct SDL_PrivateGLData *)
				malloc((sizeof *device->gl_data));
	}
	if ( (device == NULL) || (device->hidden == NULL) ||
	                         (device->gl_data == NULL) ) {
		D(bug("Unable to create video device!\n"));
		SDL_OutOfMemory();
		CGX_DeleteDevice(device);
		return(0);
	}
	memset(device->hidden, 0, sizeof(*device->hidden));
	memset(device->gl_data, 0, sizeof(*device->gl_data));

	/* Set the driver flags */
	device->handles_any_size = 1;

	/* Set the function pointers */
	device->VideoInit = CGX_VideoInit;
	device->ListModes = CGX_ListModes;
	device->SetVideoMode = CGX_SetVideoMode;
	device->ToggleFullScreen = CGX_ToggleFullScreen;
	device->UpdateMouse = CGX_UpdateMouse;
#ifdef XFREE86_XV
	device->CreateYUVOverlay = X11_CreateYUVOverlay;
#endif
	device->SetColors = CGX_SetColors;
	device->UpdateRects = NULL;
	device->VideoQuit = CGX_VideoQuit;
	device->AllocHWSurface = CGX_AllocHWSurface;
	device->CheckHWBlit = CGX_CheckHWBlit;
	device->FillHWRect = CGX_FillHWRect;
	device->SetHWColorKey = NULL;//CGX_SetHWColorKey;
	device->SetHWAlpha = NULL;
	device->LockHWSurface = CGX_LockHWSurface;
	device->UnlockHWSurface = CGX_UnlockHWSurface;
	device->FlipHWSurface = CGX_FlipHWSurface;
	device->FreeHWSurface = CGX_FreeHWSurface;
	device->SetGamma = CGX_SetGamma;
	device->GetGamma = CGX_GetGamma;
	device->SetGammaRamp = CGX_SetGammaRamp;
	device->GetGammaRamp = NULL;
#ifdef SDL_VIDEO_OPENGL
	device->GL_LoadLibrary = CGX_GL_LoadLibrary;
	device->GL_GetProcAddress = CGX_GL_GetProcAddress;
	device->GL_GetAttribute = CGX_GL_GetAttribute;
	device->GL_MakeCurrent = CGX_GL_MakeCurrent;
	device->GL_SwapBuffers = CGX_GL_SwapBuffers;
#endif
	device->SetIcon = CGX_SetIcon;
	device->SetCaption = CGX_SetCaption;
	device->IconifyWindow = NULL; /* CGX_IconifyWindow; */
	device->GrabInput = NULL /* CGX_GrabInput*/;
	device->GetWMInfo = CGX_GetWMInfo;
	device->FreeWMCursor = amiga_FreeWMCursor;
	device->CreateWMCursor = amiga_CreateWMCursor;
	device->ShowWMCursor = amiga_ShowWMCursor;
	device->WarpWMCursor = amiga_WarpWMCursor;
	device->CheckMouseMode = amiga_CheckMouseMode;
	device->InitOSKeymap = amiga_InitOSKeymap;
	device->PumpEvents = amiga_PumpEvents;

	device->free = CGX_DeleteDevice;

	return device;
}

VideoBootStrap CGX_bootstrap = {
	"CGX", "AmigaOS CyberGraphics", CGX_Available, CGX_CreateDevice
};

Uint32 MakeBitMask(_THIS,int type,int format,int *bpp, struct SDL_Surface *screen)
{
	D(if(type==0)bug("REAL pixel format: "));
   
	if(this->hidden->depth==*bpp)
	{
    	 D(bug("Pixel Format %ld\n",format));
	switch(format)
    	{
		case PIXFMT_LUT8:
			D(if(type==0)bug("LUT8\n"));
			return 0;
		case PIXFMT_BGR15:
		case PIXFMT_RGB15PC:
			switch(type)
			{
				case 0:
					D(bug("RGB15PC/BGR15\n"));
					return 31;
				case 1:
					return 992;
				case 2:
					return 31744;
			}
		case PIXFMT_RGB15:
		case PIXFMT_BGR15PC:
			switch(type)
			{
				case 0:
					D(bug("RGB15/BGR15PC\n"));
					return 31744;
				case 1:
					return 992;
				case 2:
					return 31;
			}
		case PIXFMT_BGR16PC:
			return 0;
			break;
		case PIXFMT_RGB16:

			switch(type)
			{
				case 0:
					
					return 63488;
				case 1:
					return 2016;
				case 2:
					return 31;
			}
		case PIXFMT_BGR16:
		case PIXFMT_RGB16PC:
			switch(type)
			{
				case 0:  
					//kprintf("RGB16PC/BGR16\n");
					return 0;
					//return 0b11111000; //248
					return 248; // old
				
					//return 0x7C00;
				case 1:
					return 0;
					//return 0b110000000000111;
					//return 24583;
					return 57351;  // old
				
					   
				case 2:
					return 0;
					//return 0b1111100000000; //7936
					return 7936; // old
				
			}

		case PIXFMT_RGB24:
			switch(type)
			{
				case 0:
					D(bug("RGB24/BGR24\n"));
					return 0xff0000;
				case 1:
					return 0xff00;
				case 2:
					return 0xff;
			}
		case PIXFMT_BGR24:
			switch(type)
			{
				case 0:
					D(bug("BGR24\n"));
					return 0xff;
				case 1:
					return 0xff00;
				case 2:
					return 0xff0000;
			}
		case PIXFMT_ARGB32:
		
			switch(type)
			{
				case 0:
					D(bug("ARGB32\n"));
					return 0xff0000;
				case 1:
					return 0xff00;
				case 2:
					return 0xff;
			}
		case PIXFMT_BGRA32:
		   
			switch(type)
			{
                case 0:
					D(bug("BGRA32\n"));
					if (screen && (screen->flags & SDL_HWSURFACE)){return 0x00ff00;}	
                    return 0xff000000;						
				case 1:
					if (screen && (screen->flags & SDL_HWSURFACE)){return 0xff0000;} //green
					return 0x00ff0000; 
				case 2:
					if (screen && (screen->flags & SDL_HWSURFACE))return 0xff000000;	//blue
					return 0x0000ff00;
					
            }
		case PIXFMT_RGBA32:
			
			switch(type)
			{
				case 0:
					D(bug("RGBA32\n"));
					return 0xff000000;
				case 1:
					return 0xff0000;
				case 2:
					return 0xff00;
			}
		default:
			D(bug("Unknown pixel format! Default to 24bit\n"));
			return (Uint32) (255<<(type*8));
	}
	}
	else
	{
		D(if(type==0)bug("DIFFERENT from screen.\nAllocated screen format: "));

		switch(*bpp)
		{
			case 32:
				D(if(type==0) bug("RGBA32\n"));
				switch(type)
				{
					case 0:
						return 0xff000000;
					case 1:
						return 0xff0000;
					case 2:
						return 0xff00;
				}
				break;
			case 24:
use_truecolor:
				switch(type)
				{
					case 0:
						D(bug("RGB24\n"));
						return 0xff0000;
					case 1:
						return 0xff00;
					case 2:
						return 0xff;
				}
			case 16:
			case 15:
				D(if(type==0) bug("Not supported, switching to 24bit!\n"));
				*bpp=24;
			
				goto use_truecolor;
				break;
			default:
				D(if(type==0)bug("This is a chunky display\n"));
// For chunky display mask is always 0;
				return 0;
		}
	}
	return 0;
}

static int CGX_VideoInit(_THIS, SDL_PixelFormat *vformat)
{
	int i;
    char *test;
    struct Library *RTGBase;
    int bpp;
    
    bpp=vformat->BitsPerPixel;


	D(bug("VideoInit... Opening libraries\n"));

	if(!IntuitionBase) {
		if( !(IntuitionBase=(struct IntuitionBase *)OpenLibrary("intuition.library",39L))) {
			SDL_SetError("Couldn't open intuition V39+");
			return -1;
		}
	}

	if(!GfxBase) {
		if( !(GfxBase=(struct GfxBase *)OpenLibrary("graphics.library",39L))) {
			SDL_SetError("Couldn't open graphics V39+");
			return -1;
		}
	}

	if(!CyberGfxBase) {
		if( !(CyberGfxBase=OpenLibrary("cybergraphics.library",40L))) {
			SDL_SetError("Couldn't open cybergraphics.");
			return(-1);
		}
	}

// enable AMMX for Apollo Core
#ifdef APOLLO_BLIT
	if( !Apollo_EnableAMMX() )
	{
		/* abort for Apollo builds -> disabled for now, current code is m68k clean */
#if 0
		struct EasyStruct myES =
		{
		        sizeof(struct EasyStruct),
			0,
	 		"Amiga SDL Error",
		        "Cannot initialize SDL CGXVideo on this computer\nThis is an AMMX SDL build for Apollo Core\nand AMMX could not be detected.",
		        "Abort",
	        };
		EasyRequest(NULL, &myES, NULL );

		SDL_SetError("Couldn't find active Apollo Core AMMX.");
		return(-1);
#endif
	}
#endif

// check if P96 is present and if we don't need the fix
#ifndef AROS
    test = getenv("USE_P96_FIX");
    if(test && *test != '0') {
        if(RTGBase=OpenLibrary("libs:picasso96/rtg.library",0L)) {
            extern int use_picasso96;

            CloseLibrary(RTGBase);
            use_picasso96=1;
        }
    }
#endif
    
	D(bug("Library intialized, locking screen...\n"));

	SDL_Display = LockPubScreen(NULL);

	if ( SDL_Display == NULL ) {
		D(bug("Cannot lock display...\n"));
		SDL_SetError("Couldn't lock the display");
		return(-1);
	}

	D(bug("Checking if we are using a CGX native display...\n"));
   
	if(!IsCyberModeID(GetVPModeID(&SDL_Display->ViewPort)))
	{  //selden reach
		
		Uint32 okid = INVALID_ID;
		if (bpp == 32)
		{
			UWORD pixfmt[] ={PIXFMT_BGRA32,-1};
		  unsigned long *ret;
          struct CyberModeNode * cnode;
		  ret = AllocCModeListTags(CYBRMREQ_MinWidth,SDL_Display->Width,CYBRMREQ_MinHeight,SDL_Display->Height,
			  CYBRMREQ_MaxWidth,SDL_Display->Width+1000,CYBRMREQ_MaxHeight,SDL_Display->Height+1000,
			  CYBRMREQ_MaxDepth,bpp,CYBRMREQ_MinDepth,bpp,CYBRMREQ_CModelArray,&pixfmt);
			  if (ret)
			  {cnode = *ret;
               if(cnode)okid = cnode->DisplayID;
			  }
		}
		if (bpp == 16)
		{
			UWORD pixfmt[] ={PIXFMT_RGB16,-1};
		  unsigned long *ret;
          struct CyberModeNode * cnode;
		  ret = AllocCModeListTags(CYBRMREQ_MinWidth,SDL_Display->Width,CYBRMREQ_MinHeight,SDL_Display->Height,
			  CYBRMREQ_MaxWidth,SDL_Display->Width+1000,CYBRMREQ_MaxHeight,SDL_Display->Height+1000,
			  CYBRMREQ_MaxDepth,bpp,CYBRMREQ_MinDepth,16,CYBRMREQ_CModelArray,&pixfmt);
			  if (ret)
					{cnode = *ret;
					if (cnode)okid = cnode->DisplayID;
					}
		}
		D(bug("%screenmode ID%ld\n",okid));
		if (okid == INVALID_ID)
		{
		okid=BestCModeIDTags(CYBRBIDTG_NominalWidth,SDL_Display->Width,
				CYBRBIDTG_NominalHeight,SDL_Display->Height,
				CYBRBIDTG_Depth,bpp,
				TAG_DONE);
		}
        

		UnlockPubScreen(NULL,SDL_Display);

		GFX_Display=NULL;
        
		if(okid!=INVALID_ID)
		{  //selden reach
			
			GFX_Display=OpenScreenTags(NULL,
									//SA_Width,SDL_Display->Width,
									//SA_Height,SDL_Display->Height,
									SA_Title,"SDL Screen",
									SA_Depth,bpp,SA_Quiet,TRUE,
									SA_ShowTitle,FALSE,
									SA_DisplayID,okid,
									TAG_DONE);
		}
		else kprintf("Cant get a valid screenmode ID with BestCModeIDTags\n");

		if(!GFX_Display)
		{
			SDL_SetError("Unable to open a suited CGX display");
			return -1;
		}
		else SDL_Display=GFX_Display;

	}
	else GFX_Display = SDL_Display;


	/* See whether or not we need to swap pixels */

	swap_pixels = 0;

// Non e' detto che sia cosi' pero', alcune schede potrebbero gestire i modi in modo differente
	D(bug("Before GetVideoModes....\n"));

	/* Get the available video modes */
	if(CGX_GetVideoModes(this) < 0)
	    return -1;

	/* Determine the default screen depth:
	   Use the default visual (or at least one with the same depth) */

	for(i = 0; i < this->hidden->nvisuals; i++)
	{
		//kprintf(" num %ld depths here %ld \n",this->hidden->nvisuals,this->hidden->visuals[i].depth);
	    if(this->hidden->visuals[i].depth == GetCyberMapAttr(SDL_Display->RastPort.BitMap,CYBRMATTR_DEPTH))	break;
	}
	if(i == this->hidden->nvisuals) {
	    /* default visual was useless, take the deepest one instead */
	    i = 0;
	}
	SDL_Visual = this->hidden->visuals[i].visual;

//	SDL_XColorMap = SDL_DisplayColormap;

	this->hidden->depth = this->hidden->visuals[i].depth;
	//kprintf("Init: Setting screen depth to: %ld\n",this->hidden->depth));
	vformat->BitsPerPixel = this->hidden->visuals[i].depth; /* this->hidden->visuals[i].bpp; */
	D(bug("depth %ld hidden depth %ld\n",vformat->BitsPerPixel, this->hidden->depth));
	
	{
		int form;
		APTR handle;
		struct DisplayInfo info;

		if(!(handle=FindDisplayInfo(this->hidden->visuals[i].visual)))
		{
			D(bug("Unable to get visual info...\n"));
			return -1;
		}

		if(!GetDisplayInfoData(handle,(char *)&info,sizeof(struct DisplayInfo),DTAG_DISP,NULL)) {
			D(bug("Unable to get visual info data...\n"));
			return -1;
		}

		form=GetCyberIDAttr(CYBRIDATTR_PIXFMT,SDL_Visual);

		// In this case I use makebitmask in a way that I'm sure I'll get PIXFMT pixel mask

		if ( vformat->BitsPerPixel > 8 )
		{
			vformat->Rmask = MakeBitMask(this,0,form,&this->hidden->depth,0);
	  		vformat->Gmask = MakeBitMask(this,1,form,&this->hidden->depth,0);
		  	vformat->Bmask = MakeBitMask(this,2,form,&this->hidden->depth,0);
		  
		}
	}

	/* See if we have been passed a window to use */
/*	SDL_windowid = getenv("SDL_WINDOWID"); */
	SDL_windowid=NULL;

	/* Create the blank cursor */
	SDL_BlankCursor = AllocMem(16,MEMF_CHIP|MEMF_CLEAR);

	/* Fill in some window manager capabilities */
	this->info.wm_available = 1;
	this->info.hw_available = 1;
	this->info.blit_hw = 1;
	this->info.blit_hw_CC = 0;
	this->info.blit_sw = 1;
	this->info.blit_fill = 1;
	this->info.video_mem=8000; // Not always true but almost any Amiga card has this memory!

	this->hidden->same_format=0;
	SDL_RastPort=&SDL_Display->RastPort;
	/* We're done! */
	D(bug("End of CGX_VideoInit\n"));

	return(0);
}

void CGX_DestroyWindow(_THIS, SDL_Surface *screen)
{
	D(bug("Destroy Window...\n"));
   
	if ( ! SDL_windowid ) {
		/* Hide the managed window */
		int was_fullscreen=0;

		/* Clean up OpenGL */
		if ( screen ) {
		screen->flags &= ~(SDL_OPENGL|SDL_OPENGLBLIT);
		}

		if ( screen && (screen->flags & SDL_FULLSCREEN) ) {
			was_fullscreen=1;
			screen->flags &= ~SDL_FULLSCREEN;
//			CGX_LeaveFullScreen(this); tolto x crash
		}

		/* Destroy the output window */
		SDL_AmigaLockWindow();
		if ( SDL_Window ) {
			CloseWindow(SDL_Window);
			if (SDL_Window_Background)CloseWindow(SDL_Window_Background);
			SDL_Window=NULL;
			_sdl_windowaddr = 0;
            SDL_Window_Background=NULL;
			
		}
		SDL_AmigaUnlockWindow();

		/* Free the colormap entries */
		if ( SDL_XPixels ) {
			int numcolors;
			unsigned long pixel;

			if(this->screen->format&&this->hidden->depth==8&&!was_fullscreen)
			{
				numcolors = 1<<this->screen->format->BitsPerPixel;

				if(numcolors>256)
					numcolors=256;

				if(!was_fullscreen&&this->hidden->depth==8)
				{
					for ( pixel=0; pixel<numcolors; pixel++ )
					{
						if(SDL_XPixels[pixel]>=0)
							ReleasePen(GFX_Display->ViewPort.ColorMap,SDL_XPixels[pixel]);
					}
				}
			}
			free(SDL_XPixels);
			SDL_XPixels = NULL;
		}
	}
}

static void CGX_SetSizeHints(_THIS, int w, int h, Uint32 flags)
{
	if ( flags & SDL_RESIZABLE ) {
		WindowLimits(SDL_Window, 32, 32,4096,4096);
	} else {
		WindowLimits(SDL_Window, w,h,w,h);
	}
	if ( flags & SDL_FULLSCREEN ) {
		flags&=~SDL_RESIZABLE;
	}
	//else
    
	//if ( getenv("SDL_VIDEO_CENTERED") ) {
	//	int display_w, display_h;

	//	display_w = SDL_Display->Width;
	//	display_h = SDL_Display->Height;
	//	ChangeWindowBox(SDL_Window,(display_w - w /* -SDL_Window->BorderLeft-SDL_Window->BorderRight */)/2,
	//				(display_h - h /* -SDL_Window->BorderTop-SDL_Window->BorderBottom*/)/2,
	//				w/*+SDL_Window->BorderLeft+SDL_Window->BorderRight */,
	//				h/*+SDL_Window->BorderTop+SDL_Window->BorderBottom */);
	//}
}

int CGX_CreateWindow(_THIS, SDL_Surface *screen,
			    int w, int h, int bpp, Uint32 flags)
{
#if 0
	int i, depth;
	Uint32 vis;
#endif
	D(bug("CGX_CreateWindow %d\n",bpp));
    //kprintf("%ld %ld\n",screen->flags,flags);
	//kprintf("window depth %ld\n",bpp);
	/* If a window is already present, destroy it and start fresh */
	if ( SDL_Window ) {
		CGX_DestroyWindow(this, screen);
	}
 //   if ( (screen->flags&SDL_FULLSCREEN) != SDL_FULLSCREEN ) {
	//	/* There's no windowed double-buffering */
	//	screen->flags &= ~SDL_DOUBLEBUF;
	//} 
	/* See if we have been given a window id */
	if ( SDL_windowid ) {
		SDL_Window = (struct Window *)atol(SDL_windowid);
	} else {
		SDL_Window = 0;
	}

	/* Allocate the new pixel format for this video mode */
	{
		Uint32 form;
		APTR handle;
		struct DisplayInfo info;

		if(!(handle=FindDisplayInfo(SDL_Visual)))
			return -1;

		if(!GetDisplayInfoData(handle,(char *)&info,sizeof(struct DisplayInfo),DTAG_DISP,NULL))
			return -1;

		form=GetCyberIDAttr(CYBRIDATTR_PIXFMT,SDL_Visual);

		if(flags&SDL_HWSURFACE)
		{
			
			if(bpp!=this->hidden->depth)
			{
				bpp=this->hidden->depth;
				D(bug("Accel forces bpp to be equal (%ld)\n",bpp));
				
			}
		}
		if(flags&SDL_HWSURFACE)screen->flags|=SDL_HWSURFACE;
		  
 
		D(bug("BEFORE screen allocation: bpp:%ld (real:%ld)\n",bpp,this->hidden->depth));

/* With this call if needed I'll revert the wanted bpp to a bpp best suited for the display, actually occurs
   only with requested format 15/16bit and display format != 15/16bit
 */
		
        
		if ( ! SDL_ReallocFormat(screen, bpp,
				MakeBitMask(this,0,form,&bpp,screen), MakeBitMask(this,1,form,&bpp,screen), MakeBitMask(this,2,form,&bpp,screen), 0) )
			return -1;
        
		D(bug("AFTER screen allocation: bpp:%ld (real:%ld)\n",bpp,this->hidden->depth));

	}

	/* Create the appropriate colormap */
/*
	if ( SDL_XColorMap != SDL_DisplayColormap ) {
		XFreeColormap(SDL_Display, SDL_XColorMap);
	}
*/
	if ( GetCyberMapAttr(SDL_Display->RastPort.BitMap,CYBRMATTR_PIXFMT)==PIXFMT_LUT8 || bpp==8 ) {
	    int ncolors,i;
	    D(bug("XPixels palette allocation...\n"));

	    /* Allocate the pixel flags */

	    if(bpp==8)
		ncolors=256;
	    else
		ncolors = 1 << screen->format->BitsPerPixel;

	    SDL_XPixels = (Sint32 *)malloc(ncolors * sizeof(Sint32));

	    if(SDL_XPixels == NULL) {
		SDL_OutOfMemory();
		return -1;
	    }


	    for(i=0;i<ncolors;i++)
		    SDL_XPixels[i]=-1;

	    /* always allocate a private colormap on non-default visuals */
	    if(bpp==8)
		flags |= SDL_HWPALETTE;

	    if ( flags & SDL_HWPALETTE )
			screen->flags |= SDL_HWPALETTE;
	} 

	/* resize the (possibly new) window manager window */

	/* Create (or use) the X11 display window */

	if ( !SDL_windowid ) {
		int left = (SDL_Display->Width - w) / 2;
		int top = (SDL_Display->Height - h) / 2;
		if (left < 0 )left = 0;
		if (top  < 0 )top =0;
				
			if( flags & SDL_FULLSCREEN )
			{ 
				
				//SetAPen(&sc->RastPort,1); // rest of display should be black
				//RectFill(&sc->RastPort,0,0,w-1,h-1); 
				if (left !=0 || top != 0)
				{
				SDL_Window_Background = OpenWindowTags(NULL,WA_Left,0,WA_Top,0, 
											WA_Flags,WFLG_SIMPLE_REFRESH | WFLG_ACTIVATE|WFLG_RMBTRAP|WFLG_BORDERLESS|WFLG_BACKDROP
											/*|WFLG_REPORTMOUSE */,
											WA_IDCMP,0,
											//WA_Title,"SDL Backgroundwindow",
											WA_CustomScreen,(ULONG)SDL_Display,
											TAG_DONE);
				}
				if (SDL_Window_Background)
				{
				SetAPen(SDL_Window_Background->RPort,1); // rest of display should be black
				RectFill(SDL_Window_Background->RPort,0,0,SDL_Window_Background->Width-1,SDL_Window_Background->Height); 
				}
				SDL_Window = OpenWindowTags(NULL,WA_Left,left,WA_Top,top,WA_Width,w,WA_Height,h, 
											WA_Flags,WFLG_ACTIVATE|WFLG_RMBTRAP|WFLG_BORDERLESS | WFLG_REPORTMOUSE ,
											WA_IDCMP,IDCMP_RAWKEY|IDCMP_MOUSEBUTTONS|IDCMP_MOUSEMOVE |IDCMP_ACTIVEWINDOW|IDCMP_INACTIVEWINDOW,
											WA_CustomScreen,(ULONG)SDL_Display,
											TAG_DONE);
                if (SDL_Window)
				{
				 
                 
				  //ChangeWindowBox(SDL_Window,left,top,w,h);
				}
				D(bug("Opening backdrop window %ldx%ld on display %lx!\n",w,h,SDL_Display));
			}
			else
			{
				/* Create GimmeZeroZero window when OpenGL is used */
				unsigned long gzz = TRUE;
				/*if( flags & SDL_OPENGL ) {
					gzz = TRUE;
				}*/
               
				SDL_Window = OpenWindowTags(NULL,WA_Left,left,WA_Top,top,WA_InnerWidth,w,WA_InnerHeight,h,
											WA_Flags,WFLG_REPORTMOUSE|WFLG_ACTIVATE|WFLG_RMBTRAP | ((flags&SDL_NOFRAME) ? 0 : (WFLG_DEPTHGADGET|WFLG_CLOSEGADGET|WFLG_DRAGBAR | ((flags&SDL_RESIZABLE) ? WFLG_SIZEGADGET|WFLG_SIZEBBOTTOM : 0))),
											WA_IDCMP,IDCMP_RAWKEY|IDCMP_CLOSEWINDOW|IDCMP_MOUSEBUTTONS|IDCMP_NEWSIZE|IDCMP_MOUSEMOVE |IDCMP_ACTIVEWINDOW |IDCMP_INACTIVEWINDOW,
											WA_PubScreen,(ULONG)SDL_Display,
											WA_GimmeZeroZero, gzz,
														TAG_DONE);
				
				D(bug("Opening WB window of size: %ldx%ld!\n",w,h));
			}
			
        //kprintf("%lx\n",SDL_Window);
		if(!SDL_Window)
			return -1;
	}
	this->hidden->swap_bytes = 0; 
    if ((flags & SDL_OPENGL) == 0)
			{ 
				switch(GetCyberMapAttr(SDL_Window->RPort->BitMap, CYBRMATTR_PIXFMT))
				{
					//case PIXFMT_RGB15PC:
					case PIXFMT_RGB16PC:
					case PIXFMT_BGR16PC:
						this->hidden->swap_bytes = 1; 
						break;
                    case PIXFMT_BGRA32:
                        
						//this->hidden->swap_bytes = 1; 
						break;
				}
			} 
	this->hidden->BytesPerPixel=GetCyberMapAttr(SDL_Window->RPort->BitMap,CYBRMATTR_BPPIX);
	{
	    long val= this->hidden->BytesPerPixel;
	    //kprintf("bytes per pix %ld\n",val);
	}
	if(screen->flags & SDL_DOUBLEBUF)
	{
		if(SDL_RastPort=malloc(sizeof(struct RastPort)))
		{
			InitRastPort(SDL_RastPort);
			if( this->hidden->dbscrollscreen == 1 ) /* use scrolling screen for double buffer */
			{
//				SDL_RastPort->BitMap=SDL_Display->RastPort.BitMap;
				SDL_RastPort->BitMap=&this->hidden->dbitmap;
			}
			else
				SDL_RastPort->BitMap=this->hidden->SB[1]->sb_BitMap;
			
		}
		else
			return -1;
	}
	else 
		SDL_RastPort=SDL_Window->RPort;
	
    
	if( !SDL_windowid ) {
	    CGX_SetSizeHints(this, w, h, flags);
		current_w = w;
		current_h = h;
	}

	/* Map them both and go fullscreen, if requested */
	if ( ! SDL_windowid ) {
		if ( flags & SDL_FULLSCREEN ) {
			screen->flags |= SDL_FULLSCREEN;
			currently_fullscreen=1;
			
			//CGX_EnterFullScreen(this); //Ci siamo gia'!
		} else {
			screen->flags &= ~SDL_FULLSCREEN;
		}
	}
	screen->w = w;
	screen->h = h;
	screen->pitch = SDL_CalculatePitch(screen);
	if (CGX_ResizeImage(this, screen, flags) <0 )return -1;

	/* Make OpenGL Context if needed*/
	if(flags & SDL_OPENGL) {
		
		if(this->gl_data->gl_active == 0) {
			if(CGX_GL_Init(this) < 0)
				return -1;
			else
				screen->flags |= SDL_OPENGL;
		}
		else {
			if(CGX_GL_Update(this) < 0)
				return -1;
			else
				screen->flags |= SDL_OPENGL;
		}
	}
	_sdl_windowaddr = SDL_Window;
	//kprintf("bit %ld byte / Pixel %ld render buffer addr %lx \n",screen->format->BitsPerPixel,this->hidden->BytesPerPixel,screen->pixels);
	return 0;
}

int CGX_ResizeWindow(_THIS,
			SDL_Surface *screen, int w, int h, Uint32 flags)
{
	int retval = -1;
	D(bug("CGX_ResizeWindow\n"));
	if ( ! SDL_windowid ) {
		/* Resize the window manager window */
		CGX_SetSizeHints(this, w, h, flags);
		current_w = w;
		current_h = h;

		ChangeWindowBox(SDL_Window,SDL_Window->LeftEdge ,SDL_Window->TopEdge, w+SDL_Window->BorderLeft+SDL_Window->BorderRight,
					h+SDL_Window->BorderTop+SDL_Window->BorderBottom);

		screen->w = w;
		screen->h = h;
		screen->pitch = SDL_CalculatePitch(screen);
		 retval = CGX_ResizeImage(this, screen, flags);
	}
	return(retval);
}

static SDL_Surface *CGX_SetVideoMode(_THIS, SDL_Surface *current,
				int width, int height, int bpp, Uint32 flags)
{
	Uint32 saved_flags;
	int needcreate=0;
	D(bug("CGX_SetVideoMode current:%lx\n",current));
#ifdef APOLLO_BLIT
	if( !Apollo_AMMXon )
		flags &= ~SDL_DOUBLEBUF;
#endif
	//printf("flags %x dbfl %x\n",flags,SDL_DOUBLEBUF);

	/* Lock the event thread, in multi-threading environments */
	SDL_Lock_EventThread();
	SDL_AmigaLockWindow();

// Check if the window needs to be closed or can be resized

	if( (flags&SDL_FULLSCREEN) || (current && current->flags&SDL_FULLSCREEN && !(flags&SDL_FULLSCREEN)))
		needcreate=1;

// Check if we need to close an already existing videomode...
	if(current && current->flags&SDL_FULLSCREEN && !(flags&SDL_FULLSCREEN)) {
		unsigned long i;
		D(bug("Destroying image, window & screen!\n"));

		CGX_DestroyImage(this,current);
		CGX_DestroyWindow(this,current);
		DestroyScreen(this);
		GFX_Display=SDL_Display=LockPubScreen(NULL);

		bpp=this->hidden->depth=GetCyberMapAttr(SDL_Display->RastPort.BitMap,CYBRMATTR_DEPTH);
        //kprintf("Set Video mode depth %ld\n",bpp);
		for ( i = 0; i < this->hidden->nvisuals; i++ ) {
			if ( this->hidden->visuals[i].depth == bpp ) /* era .depth */
				break;
		}
		if ( i == this->hidden->nvisuals ) {
			SDL_SetError("No matching visual for requested depth");
			return NULL;	/* should never happen */
		}
		SDL_Visual = this->hidden->visuals[i].visual;

		D(bug("Setting screen depth to: %ld\n",this->hidden->depth));

	}
	/* Check the combination of flags we were passed */
	if ( flags & SDL_FULLSCREEN ) {
		int i;
            int swidth,sheight;
	    int dbscroll = 1; /* def: we can use a scrolling screen for doublebuffer */

#ifndef APOLLO_BLIT
		dbscroll = 0; /* disable except from Apollo SAGA driver (for now, needs testing on other HW) */
#endif
		if( flags&SDL_DOUBLEBUF)
		{
			//printf("doublebuf requested\n");
		}
		else
		{
			dbscroll = 0;
			//printf("doublebuf not set\n");
		}
		/* Clear fullscreen flag if not supported */
		if ( SDL_windowid ) {
			flags &= ~SDL_FULLSCREEN;
		}
		else if(current && current->flags&SDL_FULLSCREEN ) {
			if(current->w!=width ||
				current->h!=height ||
				(this->hidden && this->hidden->depth!=bpp))
			{
				D(bug("Deleting previous window...\n"));
				CGX_DestroyImage(this,current);
				CGX_DestroyWindow(this,current);
				DestroyScreen(this);
				goto buildnewscreen;
			}
		}
		else
buildnewscreen:
		{
			Uint32 okid=INVALID_ID;
		/*if ((flags & SDL_OPENGL) && (bpp == 24))
		{
			bpp = 32;
		}*/
			if (bpp == 32)
		{
			UWORD pixfmt[] ={PIXFMT_BGRA32,-1};
		  unsigned long *ret;
          struct CyberModeNode * cnode;
		  ret = AllocCModeListTags(CYBRMREQ_MinWidth,width,CYBRMREQ_MinHeight,height,
			  CYBRMREQ_MaxWidth,width+1000,CYBRMREQ_MaxHeight,height+1000,
			  CYBRMREQ_MaxDepth,bpp,CYBRMREQ_MinDepth,32,CYBRMREQ_CModelArray,(&pixfmt),TAG_END);
			  if (ret)
					{cnode = *ret;
					if (cnode)okid = cnode->DisplayID;
					}
		}
		//if (bpp == 16)
//		{
//			UWORD pixfmt[] ={PIXFMT_RGB16,-1};
//		  unsigned long *ret;
//          struct CyberModeNode * cnode;
//		  ret = AllocCModeListTags(CYBRMREQ_MinWidth,width,CYBRMREQ_MinHeight,height,
//			  CYBRMREQ_MaxWidth,width+1000,CYBRMREQ_MaxHeight,height+1000,
//			  CYBRMREQ_MaxDepth,bpp,CYBRMREQ_MinDepth,16,CYBRMREQ_CModelArray,&pixfmt,TAG_END);
//			  if (ret)
//					{cnode = *ret;
//					if (cnode)okid = cnode->DisplayID;
//					}
//		if (!IsCyberModeID(okid))okid = INVALID_ID;
//		kprintf(" RGB16  Mode ID %lx\n",okid);
//		}
		
		
		if (okid == INVALID_ID)
		{
				okid=BestCModeIDTags(CYBRBIDTG_NominalWidth,width,
				CYBRBIDTG_NominalHeight,height,
				CYBRBIDTG_Depth,bpp,
				TAG_DONE);
			D(bug("Mode ID %lx\n",okid));		
		}  


		GFX_Display=NULL;
		this->hidden->dbscrollscreen = 0; /* no doublebuffer scroll screen */
			D(bug("Opening screen...\n"));
			if(okid!=INVALID_ID)
			{
				struct DimensionInfo dinfo;
               
                  int  result = GetDisplayInfoData(0,&dinfo,sizeof (struct DimensionInfo),DTAG_DIMS,okid);
						if (result)
						{
					          swidth=dinfo.Nominal.MaxX;
						  sheight=dinfo.Nominal.MaxY;

						  if( dinfo.MaxRasterHeight < 2*height )
						  {
							dbscroll = 0; /* we can't scroll for double buffer */
						  }
						}
				//printf("DBScroll %d\n",dbscroll);
				if( dbscroll )
				{
				 /* open at native resolution, 2 times the native height */
				 GFX_Display=OpenScreenTags(NULL,
				 				SA_Top,0,
								SA_Left,0,
								SA_Width,swidth+1,
								SA_Height,sheight*2+2,
								SA_Title,"AMMX SDL Screen",
//								SA_Quiet,TRUE,SA_ShowTitle,FALSE,
								SA_AutoScroll,FALSE,
								SA_Draggable,FALSE,
								SA_Depth,bpp,
								SA_Type,CUSTOMSCREEN,
								SA_Exclusive,TRUE,
								SA_DisplayID,okid,
								TAG_DONE);
				 this->hidden->dbpos = 0;
				 this->hidden->dbheight = sheight;
				 //printf("open double buffered screen %x\n",(int)GFX_Display);
				}
				if( !GFX_Display )
				{
				 dbscroll = 0;
				 GFX_Display=OpenScreenTags(NULL,
								//SA_Width,width,
								//SA_Height,height,
								SA_Title,"SDL Screen",
								SA_Quiet,TRUE,SA_ShowTitle,FALSE,
								SA_Depth,bpp,
								SA_DisplayID,okid,
								TAG_DONE);
				}
				if( dbscroll )
				 this->hidden->dbscrollscreen = 1; /* use scrolling screen for double buffer */
			}
			else kprintf("2: Cant get a valid screenmode ID with BestCModeIDTags\n");
			if(!GFX_Display) {
				GFX_Display=SDL_Display;
				flags &= ~SDL_FULLSCREEN;
				flags &= ~SDL_DOUBLEBUF;
				kprintf("Screen cant open \n");
			}
			else {
				UnlockPubScreen(NULL,SDL_Display);
				SDL_Display=GFX_Display;
	
				D(bug("Screen opened.\n"));
                
				if(flags&SDL_DOUBLEBUF) 
				{
				 if( this->hidden->dbscrollscreen )
				 {
					D(bug("Initialize double buffered scrolling screen\n"));
					/* copy bitmap */
					memcpy( &this->hidden->dbitmap,GFX_Display->RastPort.BitMap, sizeof( struct BitMap ));
					/* move secondary bitmap further down */
					this->hidden->dbitmap.Planes[0] += this->hidden->dbitmap.BytesPerRow * this->hidden->dbheight;
					this->hidden->dbuffer=2;
					current->flags|=SDL_DOUBLEBUF;
				 }
				 else /* if( this->hidden->dbscrollscreen ) */
				 {
					int ok=0;
					D(bug("Start of DBuffering allocations...\n"));

					if(this->hidden->SB[0]=AllocScreenBuffer(SDL_Display,NULL,SB_SCREEN_BITMAP)) {

						if(this->hidden->SB[1]=AllocScreenBuffer(SDL_Display,NULL,0L)) {
							extern struct MsgPort *safeport,*dispport;

							safeport=CreateMsgPort();
							dispport=CreateMsgPort();

							if(!safeport || !dispport) {
								if(safeport) {
									DeleteMsgPort(safeport);
									safeport=NULL;
								}
								if(dispport) {
									DeleteMsgPort(dispport);
									dispport=NULL;
								}
								FreeScreenBuffer(SDL_Display,this->hidden->SB[0]);
								FreeScreenBuffer(SDL_Display,this->hidden->SB[1]);
							}
							else {
								extern ULONG safe_sigbit,disp_sigbit;
								int i;

								safe_sigbit=1L<< safeport->mp_SigBit;
								disp_sigbit=1L<< dispport->mp_SigBit;

								for(i=0;i<2;i++) {
									this->hidden->SB[i]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort=safeport;
									this->hidden->SB[i]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort=dispport;
								}

								ok=1;
								kprintf("Dbuffering enabled!\n");
								this->hidden->dbuffer=1;
								current->flags|=SDL_DOUBLEBUF;
							}
						}
						else {
							FreeScreenBuffer(SDL_Display,this->hidden->SB[1]);
							this->hidden->SB[0]=NULL;
						}
					}

					if(!ok)
						flags&=~SDL_DOUBLEBUF;
				 } /* if( this->hidden->dbscrollscreen ) */
				}
			}

			if(GetCyberMapAttr(SDL_Display->RastPort.BitMap,CYBRMATTR_DEPTH)==bpp)
				this->hidden->same_format=1;
		}
        
		bpp=this->hidden->depth=GetCyberMapAttr(SDL_Display->RastPort.BitMap,CYBRMATTR_DEPTH);
		//kprintf("Set Video mode depth %ld\n",bpp);
		D(bug("Setting screen depth to: %ld\n",this->hidden->depth));

		for ( i = 0; i < this->hidden->nvisuals; i++ )
			if ( this->hidden->visuals[i].depth == bpp ) /* era .depth */
				break;

		if ( i == this->hidden->nvisuals ) {
			SDL_SetError("No matching visual for requested depth");
			return NULL;	/* should never happen */
		}
		//SDL_Visual = this->hidden->visuals[i].visual;
        SDL_Visual = this->hidden->visuals[i].visual; //no changecreen
	}

	/* Set up the X11 window */
	saved_flags = current->flags;

	if (SDL_Window && (saved_flags&SDL_OPENGL) == (flags&SDL_OPENGL)
	    && bpp == current->format->BitsPerPixel && !needcreate) {
		if (CGX_ResizeWindow(this, current, width, height, flags) < 0) {
			current = NULL;
			goto done;
		}
	} else {
		
		if (CGX_CreateWindow(this,current,width,height,bpp,flags) < 0) {
			current = NULL;
			goto done;
		}
	}
 
#if 0
	/* Set up the new mode framebuffer */
	if ( ((current->w != width) || (current->h != height)) ||
             ((saved_flags&SDL_OPENGL) != (flags&SDL_OPENGL)) ) {
		current->w = width;
		current->h = height;
		current->pitch = SDL_CalculatePitch(current);
		CGX_ResizeImage(this, current, flags);
	}
#endif

	current->flags |= (flags&SDL_RESIZABLE); // Resizable only if the user asked it

  done:
	Delay(1);
	/* Release the event thread */
	SDL_Unlock_EventThread();
	SDL_AmigaUnlockWindow();

	/* We're done! */
	return(current);
}

static int CGX_ToggleFullScreen(_THIS, int on)
{
	Uint32 event_thread;

	/* Don't switch if we don't own the window */
	if ( SDL_windowid ) {
		return(0);
	}

	/* Don't lock if we are the event thread */
	event_thread = SDL_EventThreadID();
	if ( event_thread && (SDL_ThreadID() == event_thread) ) {
		event_thread = 0;
	}
	if ( event_thread ) {
		SDL_Lock_EventThread();
	} 
	if ( on ) {
		this->screen->flags |= SDL_FULLSCREEN;
		
		CGX_EnterFullScreen(this);
		SDL_SetVideoMode(this->screen->w - this->offset_x, this->screen->h - this->offset_y,this->hidden->depth,this->screen->flags);
		//SDL_FillRect(this->screen, NULL, 0);
	} else {
		this->screen->flags &= ~SDL_FULLSCREEN;
		CGX_LeaveFullScreen(this);
		SDL_SetVideoMode(this->screen->w , this->screen->h,this->hidden->depth , this->screen->flags);

	}

	CGX_RefreshDisplay(this);
	if ( event_thread ) {
		SDL_Unlock_EventThread();
	}
    
	SDL_ResetKeyboard();
   
	return(1);
}

static void SetSingleColor(Uint32 fmt, unsigned char r, unsigned char g, unsigned char b, unsigned char *c)
{
	
	switch(fmt)
	{
		case PIXFMT_BGR15:
		case PIXFMT_RGB15PC:
			{
				Uint16 *t=(Uint16 *)c;
				*t=(r>>3) | ((g>>3)<<5) | ((b>>3)<<10) ;
			}
			break;
		case PIXFMT_RGB15:
		case PIXFMT_BGR15PC:
			{
				Uint16 *t=(Uint16 *)c;
				*t=(b>>3) | ((g>>3)<<5) | ((r>>3)<<10) ;
			}
			break; 
		case PIXFMT_BGR16PC:
		case PIXFMT_RGB16:
			{ 
				Uint16 *t=(Uint16 *)c;
				*t=(b>>3) | ((g>>2)<<5) | ((r>>3)<<11) ;
			}
			break;
		case PIXFMT_BGR16:
		case PIXFMT_RGB16PC:
			{
				Uint16 *t=(Uint16 *)c;
				*t=(r>>3) | ((g>>2)<<5) | ((b>>3)<<11) ;
			}
			break;
		case PIXFMT_RGB24:
			c[0]=r;
			c[1]=g;
			c[2]=b;
			c[3]=0;
			break;
		case PIXFMT_BGR24:
			c[0]=b;
			c[1]=g;
			c[2]=r;
			c[3]=0;
			break;
		case PIXFMT_ARGB32:
			c[0]=0;
			c[1]=r;
			c[2]=g;
			c[3]=b;
			break;
		case PIXFMT_BGRA32:
        
			c[0]=b;
			c[1]=g;
			c[2]=r;
			c[3]=0;
			break;
		case PIXFMT_RGBA32:
			c[0]=r;
			c[1]=g;
			c[2]=b;
			c[3]=0;
			break;

		default:
			D(bug("Error, SetSingleColor with PIXFMT %ld!\n",fmt));
	}
}

/* Update the current mouse state and position */
static void CGX_UpdateMouse(_THIS)
{
	/* Lock the event thread, in multi-threading environments */
	SDL_Lock_EventThread();

	if(currently_fullscreen)
	{
			SDL_PrivateAppActive(1, SDL_APPMOUSEFOCUS);
			SDL_PrivateMouseMotion(0, 0, SDL_Display->MouseX, SDL_Display->MouseY);
	}
	else
	{
		if(	SDL_Display->MouseX>=(SDL_Window->LeftEdge/*+SDL_Window->BorderLeft*/) && SDL_Display->MouseX<(SDL_Window->LeftEdge+ SDL_Window->Width/*-SDL_Window->BorderRight*/) &&
			SDL_Display->MouseY>=(SDL_Window->TopEdge/*+SDL_Window->BorderLeft*/) && SDL_Display->MouseY<(SDL_Window->TopEdge+SDL_Window->Height/*-SDL_Window->BorderBottom*/)
			)
		{
			SDL_PrivateAppActive(1, SDL_APPMOUSEFOCUS);
			SDL_PrivateMouseMotion(0, 0, SDL_Display->MouseX-SDL_Window->LeftEdge/*-SDL_Window->BorderLeft*/,
										SDL_Display->MouseY-SDL_Window->TopEdge/*-SDL_Window->BorderTop*/);
		}
		else
		{
			SDL_PrivateAppActive(0, SDL_APPMOUSEFOCUS);
		}
	}
	SDL_Unlock_EventThread();
}

static int CGX_SetColors(_THIS, int firstcolor, int ncolors, SDL_Color *colors)
{
	int      i;

	/* Check to make sure we have a colormap allocated */

	/* It's easy if we have a hidden colormap */
	if ( (this->screen->flags & SDL_HWPALETTE) && currently_fullscreen )
	{
		ULONG  xcmap[256*3+2];

		xcmap[0]=(ncolors<<16);
		xcmap[0]+=firstcolor;

//		D(bug("Setting %ld colors from color %ld on an HWPALETTE screen\n",ncolors,firstcolor));

// per ora ho sostituito questo gli altri casi sono ancora da gestire

		for ( i=0; i<ncolors; i++ ) {
			xcmap[i*3+1] = colors[i].r<<24;
			xcmap[i*3+2] = colors[i].g<<24;
			xcmap[i*3+3] = colors[i].b<<24;
		}
		xcmap[ncolors*3+1]=0;
		LoadRGB32(&GFX_Display->ViewPort,xcmap);
	} else {
// XPixels are not needed on 8bit screen with hwpalette
		unsigned long pixel;

		if ( SDL_XPixels == NULL ) {
			D(bug("SetColors without colormap!"));
			return(0);
		}

		if(this->hidden->depth==8)
		{
//			D(bug("Obtaining %ld colors on the screen\n",ncolors));

			for ( pixel=firstcolor; pixel<ncolors+firstcolor; pixel++ ) {
				if(SDL_XPixels[pixel]>=0)
					ReleasePen(GFX_Display->ViewPort.ColorMap,SDL_XPixels[pixel]);
			}

		/* Try to allocate all the colors */
			for ( i=0; i<ncolors; i++ ) {
				SDL_XPixels[i+firstcolor]=ObtainBestPenA(GFX_Display->ViewPort.ColorMap,colors[i].r<<24,colors[i].g<<24,colors[i].b<<24,NULL);
			}
		}
		else
		{
#ifndef USE_CGX_WRITELUTPIXEL
			Uint32 fmt;
//			D(bug("Preparing a conversion pixel table...\n"));

			fmt=GetCyberMapAttr(SDL_Display->RastPort.BitMap,CYBRMATTR_PIXFMT);

			for(i=0;i<ncolors;i++) {
				SetSingleColor(fmt,colors[i].r,colors[i].g,colors[i].b,(unsigned char *)&SDL_XPixels[firstcolor+i]);
			}
#else
//			D(bug("Executing XPixel(%lx) remapping: (from %ld, %ld colors) first: r%ld g%ld b%ld\n",SDL_XPixels,firstcolor,ncolors,colors[firstcolor].r,colors[firstcolor].g,colors[firstcolor].b));
			for(i=0;i<ncolors;i++)
				SDL_XPixels[i+firstcolor]=(colors[i].r<<16)+(colors[i].g<<8)+colors[i].b;
#endif
		}
	}

// Actually it cannot fail!

	return 1;
}

/* Note:  If we are terminated, this could be called in the middle of
   another SDL video routine -- notably UpdateRects.
*/
static void CGX_VideoQuit(_THIS)
{
	/* Shutdown everything that's still up */
	/* The event thread should be done, so we can touch SDL_Display */
	D(bug("CGX_VideoQuit\n"));
    
	if ( SDL_Display != NULL ) {
		/* Clean up OpenGL */
		if(this->gl_data->gl_active == 1) {
			CGX_GL_Quit(this);
		}
		/* Start shutting down the windows */
		D(bug("Destroying image...\n"));
		CGX_DestroyImage(this, this->screen);
		D(bug("Destroying window...\n"));
		CGX_DestroyWindow(this, this->screen);
// Otherwise SDL_VideoQuit will try to free it!
		SDL_VideoSurface=NULL;

		CGX_FreeVideoModes(this);

		/* Free that blank cursor */
		if ( SDL_BlankCursor != NULL ) {
			FreeMem(SDL_BlankCursor,16);
			SDL_BlankCursor = NULL;
		}

		/* Close the X11 graphics connection */
		this->hidden->same_format=0;

		D(bug("Destroying screen...\n"));

		if ( GFX_Display != NULL )
			DestroyScreen(this);

		/* Close the X11 display connection */
		SDL_Display = NULL;

		/* Unload GL library after X11 shuts down */
	}

	D(bug("Closing libraries...\n"));

	if( CyberGfxBase) {
		CloseLibrary(CyberGfxBase);
		CyberGfxBase=NULL;
	}

	if (IntuitionBase) {
		CloseLibrary((struct Library *)IntuitionBase);
		IntuitionBase=NULL;
	}
	if (GfxBase) {
		CloseLibrary((struct Library *)GfxBase);
		GfxBase=NULL;
	}

	if ( this->screen && (this->screen->flags & SDL_HWSURFACE) ) {
		/* Direct screen access, no memory buffer */
		this->screen->pixels = NULL;
	}
	D(bug("End of CGX_VideoQuit.\n"));

}
 
