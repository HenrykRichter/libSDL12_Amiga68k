; ------------------------------------------------------------------------------
; | Apollo AMMX blitting functions                                             |
; | Henryk Richter <henryk.richter@gmx.net>                                    |
; | All rights reserved                                                        |
; ------------------------------------------------------------------------------
;

; enable debugging code block (unused right now)
DEBUG	EQU	0
;
; AMMX usage instruction set fuse, "burned" on first use (i.e. replace by appropriate branch)
; this means, the code will branch to 68k generic code or AMMX code
; of this define is FALSE (=0), the code will be Apollo only
DOFUSE68	EQU	1
FUSE68CHECK	macro
	tst.b	_Apollo_AMMX2on	;have Apollo AMMX2 ? (AMMX1: _Apollo_AMMXon
		endm

	ifne	DOFUSE68
	XREF	_Apollo_AMMXon		;0.b = Off, 1.b = On
	XREF	_Apollo_AMMX2on
_LVOCacheClearU     EQU   -636		;yes, yes I know...
	endc


	XDEF	_ApolloKeyARGBtoARGB		;TODO
	XDEF	_ApolloKeyRGB565toRGB565	;


	machine	ac68080

;------------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------------
;
; Macros
;
;------------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------------

; macro stuff
;  CONVHEAD4P
;   move (a0)+,(a1)+    ;number of bytes according to input/output pixel formats
;  CONVMID4P
;   move.b (a0)+,(a1)+ ; single conversion if number of pixels is not a multiple of 4
;  CONVEND4P
;  CONVLINEEND

; conversion header (set up variables and define outer loop)
; variables used/modified in the macros
;  D5 - horizontal loop count (in pixels)
;  D4 - trashed in CONVHEAD4P
;  D0 - trashed in CONVHEAD4P
;
CONVHEAD4P	macro
	; 4 pixels per loop + remaining pixels afterwards - change size to match
	;
	; 4 instructions, one Apollo cycle
	moveq	#-4,d5		;F $fffffffc
	and.l	d0,d5		;F clear lower 2 bits
	moveq	#3,d4		;F
	and.l	d4,d0		;F keep lower 2 bits
	 sub.l	d0,d5		;total width - (0,1,2,3) -> simplifies loop
.yloop:
	move.l	d5,d0
.xloop:
	ble.s	.xleftover	;less than 4 pixels remaining ?
		endm

	;(a0)+,(a1)+ in here

CONVMID4P	macro
	subq.l	#4,d0		;
	bra.s	.xloop
.xleftover:	
	beq.s	.xnoleftover
				;1...3 pixels left over (if width is not a multiple of 4
		endm

	;(a0)+,(a1)+ in here

CONVEND4P	macro
	addq.l	#1,d0
	bra.s	.xleftover	;leftover loop

.xnoleftover:			;end of x loop
		endm

CONVLINEEND	macro
	subq.l	#1,d1

	adda.l	d2,a0		;add skip values
	adda.l	d3,a1

	bne.s	.yloop
		endm

; in:    -
; out:   -
; trash: D4
FUSE68	macro
	ifne	DOFUSE68
	move.w	#\1-*-2,d4	;see routine below, don't change this on it's own
	bsr.w	BurnFuse	;this is somewhat cautious but it could be more 
				;efficient with bsr.w, followed by dc.w \1-*-...
	else
	;do nothing if disabled
	endc
	endm


;------------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------------
;
; Code section
;
;------------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------------
	section	code,code

; Called once per burned fuse function
; (note: could be optimized to burn all fuses at once)
	ifne	DOFUSE68

BurnFuse:
	movem.l	d0/a0,-(sp)
	move.l	8(sp),a0	;return address (skipping the saved registers)

	FUSE68CHECK
	beq.s	.burn68k

	;
	; burn in Apollo by executing nonsense code
	;
        move.w  #$283c,-8(a0)   ;move.l #xxx,d4 ;TODO: short-range branch
        move.w  #$7A00,-2(a0)   ;moveq  #0,d5   ;fused instructions, 1 cycle lost on Apollo

	bra.s	.ret

.burn68k:
	;
	; burn in 68k Code Path
	;
	move.w	#$6000,-8(a0)	;replace "move.w #branch,d4" by "bra.w branch"
	subq.l	#8,8(sp)	;jump to the correct branch, before the BSR

	movem.l	d1/a1/a6,-(sp)
	move.l	4.w,a6
	jsr	_LVOCacheClearU(a6)
	movem.l	(sp)+,d1/a1/a6

.ret:
	movem.l	(sp)+,d0/a0
	rts

	endc


; In: A0 - iptr
;     A1 - optr
;     D2 - skipped bytes in input after "width" pixels
;     D3 - skipped bytes in output after "width" pixels
;     D0 - width in pixels to copy
;     D1 - height in pixels to copy
;     D6 - color key
;
; Out:
;     -

_ApolloKeyARGBtoARGB:

	rts

;BlitNtoNPixelAlpha

;D6 = key color
_ApolloKeyRGB565toRGB565:
	movem.l	d4-d5,-(sp)
	bra	m68kKeyRGB565toRGB565
;	FUSE68	m68kKeyRGB565toRGB565

	CONVHEAD4P		;set up d5 (trash D0,D4 but D4 is useable now)

	;AMMX assisted branch-free version

		cmp	(a0)+,d6	;pixel A
		 sne	d4
		lsl.l	#2,d4
		cmp	(a0)+,d6	;pixel B
		 sne	d4
		lsl.l	#2,d4
		cmp	(a0)+,d6	;pixel C
		 sne	d4
		lsl.l	#2,d4
		cmp	(a0)+,d6	;pixel D
		 sne	d4
		load	-8(a0),E0
		lsr.l	#6,d4		;result in D4 AABBCCDD
		 storeM	E0,D4,(a1)+	;


	CONVMID4P		;end of regular xloop

		move	(a0)+,d4
		cmp.w	d6,d4
		beq.s	.skip5
		move.w	d4,(a1)
.skip5		addq.l	#2,a1

	CONVEND4P		;end of leftover xloop
	CONVLINEEND		;y loop: advance a0/a1, loop if lines left

ApolloKeyRGB565toRGB565_end:
	movem.l	(sp)+,d4-d5
	rts


m68kKeyRGB565toRGB565:	;safe to use: D4

	CONVHEAD4P		;set up d5 (trash D0,D4 but D4 is useable now)

			;plain 68k version
			;actually not that bad, Apollo Core will 
			;fuse beq and move.w into implicit movec
RGB565CKEY	MACRO
		move	(a0)+,d4
		cmp.w	d6,d4
		beq.s	\1
		move.w	d4,(a1)
\1		addq.l	#2,a1
		endm

		RGB565CKEY	.skipP1
		RGB565CKEY	.skipP2
		RGB565CKEY	.skipP3
		RGB565CKEY	.skipP4

	CONVMID4P		;end of regular xloop

		RGB565CKEY	.skipP5

	CONVEND4P		;end of leftover xloop
	CONVLINEEND		;y loop: advance a0/a1, loop if lines left

	bra	ApolloKeyRGB565toRGB565_end

;------------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------------
;
; Data section
;
;------------------------------------------------------------------------------------------------
;------------------------------------------------------------------------------------------------

;	section	data,data
;curbank:	dc.b	0
;		cnop	0,4


	END
