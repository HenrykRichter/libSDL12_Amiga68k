Important notes:

If you dont need opengl then the agl.library need not load.you can link with -lgl_dummy and the agl.library is not load.
If you want use SDL with libnix you must link with -lsdl_libnix or you get linker error.

but: NEVER USE sdl_libnix WITH ixemul because as soon a SDL program call a unix API func(memalloc / fileaccess) in a subthread ixemul crash.
       So libsdl.a is need and the new V61 and above ixemul to run well. 

All needed files are in lib dir.
In bin dir are the files that are need that a configure script detect SDL correct with pkgconfig.copy the files to your bin path.
when you do crosscompile and have more compilers, be careful i dont know a way how more than 1 pkgconfig can work together.

build of sdl_noixemul:

activate in thread/amigaos/sdl_systhread.c the line 
#define NOIXEMUL



-----------------------------
SDL1.2.14d

H.R: AMMX code for several blitting functions, see video/apollo with runtime selection between plain 68k code and AMMX, see SDL_cgxvideo.c as well

H.R: 18-Apr-2017  _ApolloKeyRGB565toRGB565: disabled AMMX version of ColorKeying (for now, storem is not working in Gold2)

H.R: Nov-2017
  - some fixes to 32 Bit RGB variants support (not full fix, due to limitations in the cybergraphics color format specifications
    in comparison to Picasso96)
  - some fixes to the AHI sound functions

SDL1.2.14c

toggle fullscreen clear now window/screen correct

SDL 1.2.14b

on window mode when SDL Window is open or resize it is now clear with black (do not work in before version)

SDL1.2.14

For whats new in SDL 1.2.14 see the attached doc.this are amiga enhancements only

when the SDL Window is not active then the SDL app is switch to TaskPri -1. This help to make your Amiga Desktop always fast, even if a
SDL App need 100% CPU load.

If you dont like the feature you can switch it off with that enviroment variable.
 
SDL_NoLowerTaskPri 

when open a own screen sdl try first to get a RGB16 Screen or when 32 bit screen want BGRA32 screenmode.If that fail it use whats here and write a message not optimal screenmode.

Add functions to access SDL window window addr (to move/ resize or add a menu to it(usefull if you want a SDL program use with arexx and external GUI) 

SDL_AmigaLockWindow(); // to avoid that the sdl app close the window do always a look as long you access the window pointer.
  struct Window * win = SDL_AmigaWindowAddr();
if (win)  // do only something if the address is get
  {
 	........
  }
  
  SDL_AmigaUnlockWindow();



SDL1.2.13 Version2 GCC linkerlibs

*  sdl Threads can now work with ixemul, this allow much more programs get working.
   sdl for working with libnix is named as libsdl_libnix.a and attached in adchive

*  Joystick that is default on SDL Port 0 use now Amiga Port 1 so you need not remove your mouse and plug joystick in. 
    the second joystick use then Port 0

*  SDL_HWSURFACE work now on rgb16 and rgb32 bgra32 screen mode and give 2-3* more speed in defendguin and work more systemfriendly
   on a bitmap that is later copy with AOS blitbitmaprastport on GFX Card to window.So windows on top of SDL windows are now correct visible
   resize of window work now with overlays.
*  audio with more than 2 channels can play and surround is now correct convert to 2 channels. 
   on 8 bit fullscreen mode YUV overlay play now fast 256 color grey video

* ahi use Unit 3 so you can define here a low quality setting for faster speed, withot touch your default setting

* SDL opengl work now with stormmesa and quarktex(winuae HW3d opengl) and if you have Warp3d HW also it use Warp3d HW.note the limit on Voodoo3 of textures to 256*256 is 
  here, but if the game have all images in png files you can convert images to be 256*256.they look of course not so sharp after convert, but game can play fast.
  Note: need link with libgl.a(attached).If you dont want opengl, you can use libgl_dummy.a(attached)
  You need also the new includes copy to your sdl dir. 

* check if enough memory is here before allocbitmap, so it handle low memory situitions better now.

----------------------------------------------------------
this is port of SDL 1.2.13 to amigaos 68k 
This use the directory structure of SDL so a port to newer versions is easy.
The old amiga SDL 1.2.6 files are currently in the main tree, if maybe something must compare and fail under new versions. 

there are many fixes, timer and semaphore working correct, and faster speed.

A HotKey is add CTRL+ALT+H.If press only every 2. second frame is draw to GFX Card.

this give better playable results on slow amigas because a game that is written for 30 fps need now only transfer 15 fps to gfx card to work at correct speed.

IMPORTANT: Use the new includes, this work too on old SDL 

sources can download here

http://amiga.svn.sourceforge.net/viewvc/amiga/
