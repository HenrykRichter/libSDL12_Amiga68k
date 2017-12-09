# Makefile for gcc version of SDL

PREFX=/opt/netsurf/m68k-unknown-amigaos/cross
PREF=/mnt/e/usr/local/amiga/m68k-amigaos/clib

		  
CC := $(PREFX)/bin/m68k-unknown-amigaos-gcc 
AS := $(PREFX)/bin/m68k-unknown-amigaos-as
LD := $(PREFX)/bin/m68k-unknown-amigaos-ld
RL := $(PREFX)/bin/m68k-unknown-amigaos-ranlib

DEFINES= DEFINE=ENABLE_CYBERGRAPHICS DEFINE=inline=__inline  DEFINE=NO_SIGNAL_H DEFINE=HAVE_STDIO_H DEFINE=ENABLE_AHI

# DEFINE=HAVE_OPENGL
INCLUDES= IDIR=./include/SDL
#-I$(PREF)/m68k-unknown-amigaos/include -I$(PREF)/env/m68k-unknown-amigaos/sys-include
#CFLAGS = CPU=68060  CODE=FAR DATA=FAR -I. -I../include -DNOIXEMUL
#DEBUG=FULL VERBOSE

GCCFLAGS = -I/mnt/e/usr/local/amiga/clib/m68k-unknown-amigaos/env/include/ -I./include/ -I./include/SDL\
			-I/mnt/e/usr/local/amiga/clib/m68k-unknown-amigaos/env/include/  -I$(PREF)/extra/include  \
			-O3 -fomit-frame-pointer -m68020-60 -msoft-float -mnobitfield -DNO_AMIGADEBUG -DNOIXEMUL
GLFLAGS = -DSHARED_LIB -DNO_AMIGADEBUG -lamiga


GCCDEFINES = -DENABLE_CYBERGRAPHICS -DNO_SIGNAL_H -D__MEM_AMIGA -DENABLE_AHI 
#-DNO_INLINE_STDARG
  

GOBJS = audio/SDL_audio.go audio/SDL_audiocvt.go audio/SDL_mixer.go audio/SDL_wave.go audio/amigaos/SDL_ahiaudio.go \
	SDL_error.go SDL_fatal.go video/SDL_RLEaccel.go video/SDL_blit.go video/SDL_blit_0.go \
   video/SDL_blit_1.go video/SDL_blit_A.go video/SDL_blit_N.go \
   video/SDL_bmp.go video/SDL_cursor.go video/SDL_pixels.go video/SDL_surface.go video/SDL_stretch.go \
   video/SDL_yuv.go video/SDL_yuv_sw.go video/SDL_video.go \
   timer/amigaos/SDL_systimer.go timer/SDL_timer.go joystick/SDL_joystick.go \
   joystick/SDL_sysjoystick.go SDL_cdrom.go SDL_syscdrom.go events/SDL_quit.go events/SDL_active.go \
   events/SDL_keyboard.go events/SDL_mouse.go events/SDL_resize.go file/SDL_rwops.go SDL.go \
   events/SDL_events.go thread/amigaos/SDL_sysmutex.go thread/amigaos/SDL_syssem.go thread/amigaos/SDL_systhread.go thread/amigaos/SDL_thread.go \
   thread/amigaos/SDL_syscond.go video/amigaos/SDL_cgxvideo.go video/amigaos/SDL_cgxmodes.go video/amigaos/SDL_cgximage.go video/amigaos/SDL_amigaevents.go \
   video/amigaos/SDL_amigamouse.go video/amigaos/SDL_cgxgl.go video/amigaos/SDL_cgxwm.go \
   video/amigaos/SDL_cgxyuv.go video/amigaos/SDL_cgxaccel.go video/amigaos/SDL_cgxgl_wrapper.go \
   video/SDL_gamma.go SDL_lutstub.ll stdlib/SDL_stdlib.go stdlib/SDL_string.go stdlib/SDL_malloc.go stdlib/SDL_getenv.go


%.go: %.c
	$(CC) $(GCCFLAGS) $(GCCDEFINES) -o $@ -c $*.c

%.ll: %.s
	$(AS) -m68060 -o $@ $*.s

all: libSDL.a

libSDL.a: $(GOBJS)
	-rm -f libSDL.a
	ar cru libSDL.a $(GOBJS)
	$(RL) libSDL.a
	-cp libSDL.a $(PREF)/extra/lib


clean:
	-rm -f $(GOBJS)
