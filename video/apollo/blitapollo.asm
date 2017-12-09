; ------------------------------------------------------------------------------
; | Apollo AMMX blitting functions                                             |
; | Henryk Richter <henryk.richter@gmx.net>                                    |
; | All rights reserved                                                        |
; ------------------------------------------------------------------------------
;

; enable debugging code block (unused right now)
DEBUG		EQU	0
; AMMX usage instruction set fuse, "burned" on first use (i.e. replace by appropriate branch)
; this means, the code will branch to 68k generic code or AMMX code
; of this define is FALSE (=0), the code will be Apollo only
DOFUSE68	EQU	1

	ifne	DOFUSE68
	XREF	_Apollo_AMMXon		;0.b = Off, 1.b = On
_LVOCacheClearU     EQU   -636		;yes, yes I know...
	endc

	; target: RGB565
	XDEF	_ApolloARGBtoRGB565
	XDEF	_ApolloRGBtoRGB565	;
	XDEF	_ApolloBGRtoRGB565
	XDEF	_ApolloBGRAtoRGB565	;


	;target: RGB24
	XDEF	_ApolloARGBtoRGB24
	XDEF	_ApolloBGRAtoRGB24	;untested
	XDEF	_ApolloBGRtoRGB24	; 24 Bit BGR to 24 Bit RGB24 (and vice versa)

	;target: RGB32
	XDEF	_ApolloARGBtoBGRA	; 32 Bit ARGB to 32 Bit BGRA (and vice versa)

	;target: Any (byte copy, rectangular) 
	XDEF	_ApolloCopyRect

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
	ble	.xleftover	;less than 4 pixels remaining ?
		endm

	;(a0)+,(a1)+ in here

CONVMID4P	macro
	subq.l	#4,d0		;
	bra	.xloop
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

	bne	.yloop
		endm

CONVHEAD8P	macro
	; 8 pixels per loop + remaining pixels afterwards - change size to match
	;
	; 4 instructions, one Apollo cycle
	moveq	#-8,d5		;F $fffffff8
	and.l	d0,d5		;F clear lower 3 bits
	moveq	#7,d4		;F
	and.l	d4,d0		;F keep lower 3 bits
	 sub.l	d0,d5		;total width - (0,1,2,3,...,7) -> simplifies loop
.yloop:
	move.l	d5,d0
.xloop:
	ble.s	.xleftover	;less than 4 pixels remaining ?
		endm

	;(a0)+,(a1)+ in here

CONVMID8P	macro
	subq.l	#8,d0		;
	bra.s	.xloop
.xleftover:	
	beq.s	.xnoleftover
				;1...3 pixels left over (if width is not a multiple of 4
		endm

	;(a0)+,(a1)+ in here

CONVEND8P	macro	;same as 4P
	addq.l	#1,d0
	bra.s	.xleftover	;leftover loop

.xnoleftover:			;end of x loop
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

; In: A0 - iptr
;     A1 - optr
;     D2 - skipped bytes in input after "width" pixels
;     D3 - skipped bytes in output after "width" pixels
;     D0 - width in pixels to copy
;     D1 - height in pixels to copy
;
; Out:
;     -

; Called once per burned fuse function
; (note: could be optimized to burn all fuses at once)
	ifne	DOFUSE68

BurnFuse:
	movem.l	d0/a0,-(sp)
	move.l	8(sp),a0	;return address (skipping the saved registers)

	tst.b	_Apollo_AMMXon	;have Apollo AMMX ?
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



_ApolloCopyRect:
	movem.l	d4-d5,-(sp)

	FUSE68	m68kCopyRect	;trash: d4,d5

	CONVHEAD8P		;set up d5 (trash D0,D4 but D4 is useable now)

		load	(a0)+,E0 ;this doesn't copy more data per cycle than 68k version,
		store	E0,(a1)+ ;BUT! Less strain on the write buffers due to 64 Bit transfers

	CONVMID8P		;end of regular xloop

		move.b	(a0)+,(a1)+

	CONVEND8P		;end of leftover xloop
	CONVLINEEND		;y loop: advance a0/a1, loop if lines left

	movem.l	(sp)+,d4-d5
	rts


; plain m68k version
m68kCopyRect:
	CONVHEAD8P		;set up d5 (trash D0,D4 but D4 is useable now)

		move.l	(a0)+,(a1)+
		move.l	(a0)+,(a1)+

	CONVMID8P		;end of regular xloop

		move.b	(a0)+,(a1)+

	CONVEND8P		;end of leftover xloop
	CONVLINEEND		;y loop: advance a0/a1, loop if lines left

	movem.l	(sp)+,d4-d5
	rts




;------------------------------------------------------------------------------------------------
;
; 16 Bit RGB565 targets
;
;------------------------------------------------------------------------------------------------

; 32 Bit ARGB32 to 16 Bit RGB565
_ApolloARGBtoRGB565:
	movem.l	d4-d5,-(sp)

	FUSE68	m68kARGBtoRGB565	;trash: d4,d5

	CONVHEAD4P		;set up d5 (trash D0,D4 but D4 is useable now)

		;LOAD 16 Bytes = 4 Pixels
		load	(a0),E0		;ARGB ARGB
		load	8(a0),E1	;ARGB ARGB
		lea	16(a0),a0	;
		;STORE RGB565 in A1
		pack3216 E0,E1,(a1)+

	CONVMID4P		;end of regular xloop

		move.l	 (a0)+,d4
		pack3216 d4,d4,d4
		move.w	 d4,(a1)+

	CONVEND4P		;end of leftover xloop
	CONVLINEEND		;y loop: advance a0/a1, loop if lines left

	movem.l	(sp)+,d4-d5
	rts




; 24 Bit BGR24 to 16 Bit RGB565
_ApolloBGRtoRGB565:
	movem.l	d4-d5,-(sp)

	FUSE68	m68kBGRtoRGB565	;trash: d4,d5

	CONVHEAD4P		;set up d5 (trash D0,D4 but D4 is useable now)

		;LOAD 12 Bytes = 4 Pixels
		load	(a0),E0		;B0G0R0B1G1R1B2G2
		move.l	8(a0),d4	;xxxxxxxxR2B3G3R3
		lea	12(a0),a0	;

		vperm	#$CC76FFED,E0,D4,E1
		vperm	#$22105543,E0,D4,E0

		;STORE RGB565 in A1
		pack3216 E0,E1,(a1)+

	CONVMID4P		;end of regular xloop

		move.l	 (a0),d4	;xx xx xx xx B0 G0 R0 xx
		vperm	 #$66546654,d4,d4,d4 
		pack3216 d4,d4,d4
		addq.l	 #3,a0

		move.w	 d4,(a1)+

	CONVEND4P		;end of leftover xloop
	CONVLINEEND		;y loop: advance a0/a1, loop if lines left

	movem.l	(sp)+,d4-d5
	rts

; 24 Bit RGB24 to 16 Bit RGB565
_ApolloRGBtoRGB565:
	movem.l	d4-d5,-(sp)

	FUSE68	m68kRGBtoRGB565	;trash: d4,d5

	CONVHEAD4P		;set up d5 (trash D0,D4 but D4 is useable now)

		;LOAD 12 Bytes = 4 Pixels
		load	(a0),E0		;R0G0B0R1G1B1R2G2
		move.l	8(a0),d4	;xxxxxxxxB2R3G3B3
		lea	12(a0),a0	;

		vperm	#$667CDDEF,E0,D4,E1
		vperm	#$00123345,E0,D4,E0

		;STORE RGB565 in A1
		pack3216 E0,E1,(a1)+

	CONVMID4P		;end of regular xloop

		move.l	 (a0),d4	;xx xx xx xx R0 G0 B0 xx
		vperm	 #$44564456,d4,d4,d4 
		pack3216 d4,d4,d4
		addq.l	 #3,a0

		move.w	 d4,(a1)+

	CONVEND4P		;end of leftover xloop
	CONVLINEEND		;y loop: advance a0/a1, loop if lines left

	movem.l	(sp)+,d4-d5
	rts



; 32 Bit BGRA to 16 Bit RGB565
_ApolloBGRAtoRGB565:
	movem.l	d4-d5,-(sp)
	FUSE68	m68kBGRAtoRGB565	;permanently branch to plain 68k Routine if AMMX is unavailable

	CONVHEAD4P		;set up d5 (trash D0,D4 but D4 is useable now)

		load	(a0),E0		;BGRA BGRA
		load	8(a0),E1	;BGRA BGRA
		lea	16(a0),a0	;

		;PERMUTE BGRA TO ARGB
		vperm	 #$32107654,E0,E0,E0 ; ARGB ARGB
		vperm	 #$32107654,E1,E1,E1 ; ARGB ARGB

		;STORE RGB565 in A1
		pack3216 E0,E1,(a1)+

	CONVMID4P		;end of regular xloop

		movex.l	 (a0)+,d4
		;move.l	 (a0)+,d4
		;vperm	 #$76547654,d4,d4,d4 ; BGRA2ARGB
		pack3216 d4,d4,d4
		move.w	 d4,(a1)+

	CONVEND4P		;end of leftover xloop
	CONVLINEEND		;y loop: advance a0/a1, loop if lines left

	movem.l	(sp)+,d4-d5
	rts

	ifne	DOFUSE68
;
; PLAIN 68k code for cases where Apollo is unavailable
;
rgb565_rmask:	dc.l	$f800f800
rgb565_gmask:	dc.l	$00FC00FC	;after shift: $07E007E0

m68kARGBtoRGB565:	;trash: d4,d5
	movem.l		d6/d7,-(sp)
	CONVHEAD4P		;set up d5 (trash D0,D4 but D4 is useable now)


	;A R G B A R G B
	;0 1 2 3 4 5 6 7
	;R5 G6 B5
	rept	2
		move.l	1(a0),D4	;R0(8) G0(8) B0(8) n 
		moveq	#0,d6
		 move.w	D4,d6		;B0(8) n
		 move.w	5(a0),D4	;R0(8) G0(8) R1(8) G1(8)
		lsl.l	#5,d6		;0 B0(5) n n 
		move.l	rgb565_gmask(pc),d7 ;
		 clr.w	d6		;0 B0(5) 0 0
		 and.l	D4,d7		;0 G0(6) 0 G1(6)
		move.b	7(a0),d6	;0 B0(5) 0 B1(8)
		lsl.l	#3,d7		; G0(6)   G1(6)
		 and.l	rgb565_rmask(pc),D4 ;R0(5)   R1(5)
		 lsr.w	#3,d6		;0 B0(5) 0 B1(5)
		or.l	d7,D4		;R0(5)G0(6)  R1(5)G1(6)
		addq.l	#8,a0		;next input pixels
		 or.l	d6,D4		;R0(5)G0(6)B0(5) R1(5)G1(6)B1(5)
		move.l	D4,(a1)+	;store 32 Bit = 2 Pixels
	endr

	CONVMID4P		;end of regular xloop

		move.w	#$f800,D4 ;
		move.b	3(a0),d7  ; B0(8)
		 move.w	#$fc,d6   ;
		 and.w	1(a0),D4  ; R0(5)
		lsr.b	#3,d7	  ; B0(5)
		and.b	2(a0),d6  ; G0(6)<<2
		 or.b	d7,D4	  ; R0(5)...B0(5)
		 lsl.w	#3,d6	  ; G0(6)
		or.w	d6,D4	  ; R0(5)G0(6)B0(5)
		addq.l	#4,a0
		 move.w	D4,(a1)+

	CONVEND4P		;end of leftover xloop
	CONVLINEEND		;y loop: advance a0/a1, loop if lines left

	movem.l	(sp)+,d6/d7
	movem.l	(sp)+,d4-d5
	rts


; 24 Bit BGR24 to 16 Bit RGB565 (not too fast)
m68kBGRtoRGB565:
	movem.l	d6/d7,-(sp)

	CONVHEAD4P		;set up D4 (trash D0,D4 but D4 is useable now)

	rept	4
		move.w	2(a0),D4	;R n
		move.w	#$00fc,d6	;
		 and.b	1(a0),d6 	;G(6)<<2
		 and.w	#$f800,d4	;
		move.b	(a0),d7		;B(8)
		lsl.w	#3,d6		;G(6)
		 lsr.b	#3,d7		;B(5)
		 or.w	d6,D4
		addq.l	#3,a0
		or.b	d7,D4		;
		 move.w	D4,(a1)+	;
	endr

	CONVMID4P		;end of regular xloop

		move.w	2(a0),D4	;R n
		move.w	#$00fc,d6	;
		 and.b	1(a0),d6 	;G(6)<<2
		 and.w	#$f800,d4	;
		move.b	(a0),d7		;B(8)
		lsl.w	#3,d6		;G(6)
		 lsr.b	#3,d7		;B(5)
		 or.w	d6,D4
		addq.l	#3,a0
		or.b	d7,D4		;
		 move.w	D4,(a1)+	;

	CONVEND4P		;end of leftover xloop
	CONVLINEEND		;y loop: advance a0/a1, loop if lines left

	movem.l	(sp)+,d6/d7
	movem.l	(sp)+,d4-d5
	rts

; 24 Bit RGB24 to 16 Bit RGB565
m68kRGBtoRGB565:
	movem.l	d6/d7,-(sp)

	CONVHEAD4P		;set up D4 (trash D0,D4 but D4 is useable now)

	rept	4
		move.w	(a0),D4		;R n
		move.w	#$00fc,d6	;
		 and.b	1(a0),d6 	;G(6)<<2
		 and.w	#$f800,d4	;
		move.b	2(a0),d7	;B(8)
		lsl.w	#3,d6		;G(6)
		 lsr.b	#3,d7		;B(5)
		 or.w	d6,D4
		addq.l	#3,a0
		or.b	d7,D4		;
		 move.w	D4,(a1)+	;
	endr

	CONVMID4P		;end of regular xloop

		move.w	(a0),D4		;R n
		move.w	#$00fc,d6	;
		 and.b	1(a0),d6 	;G(6)<<2
		 and.w	#$f800,d4	;
		move.b	2(a0),d7	;B(8)
		lsl.w	#3,d6		;G(6)
		 lsr.b	#3,d7		;B(5)
		 or.w	d6,D4
		addq.l	#3,a0
		or.b	d7,D4		;
		 move.w	D4,(a1)+	;

	CONVEND4P		;end of leftover xloop
	CONVLINEEND		;y loop: advance a0/a1, loop if lines left

	movem.l	(sp)+,d6/d7
	movem.l	(sp)+,d4-d5
	rts



; 32 Bit BGRA to 16 Bit RGB565
m68kBGRAtoRGB565:
	movem.l	d6/d7,-(sp)

	CONVHEAD4P		;set up D4 (trash D0,D4 but D4 is useable now)

	rept	4
		move.w	2(a0),D4	;R n
		move.w	#$00fc,d6	;
		 and.b	1(a0),d6 	;G(6)<<2
		 and.w	#$f800,d4	;
		move.b	(a0),d7		;B(8)
		lsl.w	#3,d6		;G(6)
		 lsr.b	#3,d7		;B(5)
		 or.w	d6,D4
		addq.l	#4,a0
		or.b	d7,D4		;
		 move.w	D4,(a1)+	;
	endr

	CONVMID4P		;end of regular xloop

		move.w	2(a0),D4	;R n
		move.w	#$00fc,d6	;
		 and.b	1(a0),d6 	;G(6)<<2
		 and.w	#$f800,d4	;
		move.b	(a0),d7		;B(8)
		lsl.w	#3,d6		;G(6)
		 lsr.b	#3,d7		;B(5)
		 or.w	d6,D4
		addq.l	#4,a0
		or.b	d7,D4		;
		 move.w	D4,(a1)+	;

	CONVEND4P		;end of leftover xloop
	CONVLINEEND		;y loop: advance a0/a1, loop if lines left

	movem.l	(sp)+,d6/d7
	movem.l	(sp)+,d4-d5
	rts

	endc	;DOFUSE68

;------------------------------------------------------------------------------------------------
;
; 24 Bit RGB targets
;
;------------------------------------------------------------------------------------------------

; 32 Bit ARGB to 24 Bit RGB24
_ApolloARGBtoRGB24:
	movem.l	d4-d5,-(sp)
	FUSE68	m68kARGBtoRGB24	;trash: d4,d5

	CONVHEAD4P		;set up d5 (trash D0,D4 but D4 is useable now)

		;LOAD 16 Bytes = 4 Pixels
		load	(a0),E0		;A0R0G0B0 A1R1G1B1
		load	8(a0),E1	;A2R2G2B2 A3R3G3B3
		lea	16(a0),a0	;

		vperm	#$1235679a,E0,E1,E2	;R0 G0 B0 R1 G1 B1 R2 G2
		vperm	#$BDEFBDEF,E0,E1,D4	;B2 R3 G3 B3 B2 R3 G3 B3

		store	e2,(a1)		;
		move.l	d4,8(a1)	;
		lea	12(a1),a1	;


	CONVMID4P		;end of regular xloop

		move.b	 1(a0),(a1)+	;R
		move.b	 2(a0),(a1)+	;G
		move.b	 3(a0),(a1)+	;B
		addq.l	 #4,a0

	CONVEND4P		;end of leftover xloop
	CONVLINEEND		;y loop: advance a0/a1, loop if lines left

	movem.l	(sp)+,d4-d5
	rts


; 32 Bit BGRA to 24 Bit RGB24
_ApolloBGRAtoRGB24:
	movem.l	d4-d5,-(sp)
	FUSE68	m68kBGRAtoRGB24	;trash: d4,d5

	CONVHEAD4P		;set up d5 (trash D0,D4 but D4 is useable now)

		;LOAD 16 Bytes = 4 Pixels
		load	(a0),E0		;B0G0R0A0 B1G1R1A1
		load	8(a0),E1	;B2G2R2A2 B3G3R3A3
		lea	16(a0),a0	;

		vperm	#$210654A9,E0,E1,E2	;R0 G0 B0 R1 G1 B1 R2 G2
		vperm	#$8EDC8EDC,E0,E1,D4	;B2 R3 G3 B3 B2 R3 G3 B3

		store	e2,(a1)		;
		move.l	d4,8(a1)	;
		lea	12(a1),a1	;


	CONVMID4P		;end of regular xloop

		move.b	 2(a0),(a1)+	;R
		move.b	 1(a0),(a1)+	;G
		move.b	  (a0),(a1)+	;B
		addq.l	 #4,a0

	CONVEND4P		;end of leftover xloop
	CONVLINEEND		;y loop: advance a0/a1, loop if lines left

	movem.l	(sp)+,d4-d5
	rts


; 24 Bit BGR to 24 Bit RGB24 (and vice versa)
_ApolloBGRtoRGB24:
	movem.l	d4-d5,-(sp)
	FUSE68	m68kBGRtoRGB24	;trash: d4,d5

	CONVHEAD4P		;set up d5 (trash D0,D4 but D4 is useable now)

		;LOAD 12 Bytes = 4 Pixels
		load	(a0),E0		;B0 G0 R0 B1 G1 R1 B2 G2
		move.l	8(a0),d4	;xx xx xx xx R2 B3 G3 R3
		lea	12(a0),a0	;

		vperm	#$210543C7,E0,D4,E1	;R0 G0 B0 R1 G1 B1 R2 G2
		vperm	#$6FED6FED,E0,D4,D4	;B2 R3 G3 B3 B2 R3 G3 B3

		store	e1,(a1)
		move.l	d4,8(a1)
		lea	12(a1),a1	;

	CONVMID4P		;end of regular xloop

		move.b	 2(a0),(a1)+
		move.b	 1(a0),(a1)+
		move.b	  (a0),(a1)+
		addq.l	 #3,a0

	CONVEND4P		;end of leftover xloop
	CONVLINEEND		;y loop: advance a0/a1, loop if lines left

	movem.l	(sp)+,d4-d5
	rts



	ifne	DOFUSE68

; 32 Bit ARGB to 24 Bit RGB24
m68kARGBtoRGB24:
	CONVHEAD4P		;set up d5 (trash D0,D4 but D4 is useable now)
		
		;TODO: 32 bit moves
		;ARGB ARGB ARGB ARGB (4x 32 Bit)
		;RGBR GBRG BRGB      (3x 32 Bit)
	rept	4
		move.b	 1(a0),(a1)+	;R
		move.b	 2(a0),(a1)+	;G
		move.b	 3(a0),(a1)+	;B
		addq.l	 #4,a0
	endr

	CONVMID4P		;end of regular xloop

		move.b	 1(a0),(a1)+	;R
		move.b	 2(a0),(a1)+	;G
		move.b	 3(a0),(a1)+	;B
		addq.l	 #4,a0

	CONVEND4P		;end of leftover xloop
	CONVLINEEND		;y loop: advance a0/a1, loop if lines left

	movem.l	(sp)+,d4-d5
	rts


; 32 Bit BGRA to 24 Bit RGB24
m68kBGRAtoRGB24:
	CONVHEAD4P		;set up d5 (trash D0,D4 but D4 is useable now)
	
	rept 4
		move.b	 2(a0),(a1)+	;R
		move.b	 1(a0),(a1)+	;G
		move.b	  (a0),(a1)+	;B
		addq.l	 #4,a0
	endr

	CONVMID4P		;end of regular xloop

		move.b	 2(a0),(a1)+	;R
		move.b	 1(a0),(a1)+	;G
		move.b	  (a0),(a1)+	;B
		addq.l	 #4,a0

	CONVEND4P		;end of leftover xloop
	CONVLINEEND		;y loop: advance a0/a1, loop if lines left

	movem.l	(sp)+,d4-d5
	rts


; 24 Bit BGR to 24 Bit RGB24 (and vice versa)
m68kBGRtoRGB24:
	CONVHEAD4P		;set up d5 (trash D0,D4 but D4 is useable now)

	rept 4
		move.b	 2(a0),(a1)+
		move.b	 1(a0),(a1)+
		move.b	  (a0),(a1)+
		addq.l	 #3,a0
	endr

	CONVMID4P		;end of regular xloop

		move.b	 2(a0),(a1)+
		move.b	 1(a0),(a1)+
		move.b	  (a0),(a1)+
		addq.l	 #3,a0

	CONVEND4P		;end of leftover xloop
	CONVLINEEND		;y loop: advance a0/a1, loop if lines left

	movem.l	(sp)+,d4-d5
	rts

	endc	;DOFUSE68



;------------------------------------------------------------------------------------------------
;
; 32 Bit ARGB targets
;
;------------------------------------------------------------------------------------------------

; 32 Bit ARGB to 32 Bit BGRA (and vice versa)
_ApolloARGBtoBGRA:
	movem.l	d4-d5,-(sp)
	FUSE68	m68kARGBtoBGRA	;trash: d4,d5

	CONVHEAD4P		;set up d5 (trash D0,D4 but D4 is useable now)

		;LOAD 16 Bytes = 4 Pixels
		load	(a0),E0		;A0R0G0B0 A1R1G1B1
		 load	8(a0),E1	;A2R2G2B2 A3R3G3B3
		 lea	16(a0),a0	;
		vperm	#$32107654,E0,E0,E0	;BGRA BGRA
		 vperm	#$32107654,E1,E1,E1	;BGRA BGRA
		store	e0,(a1)		;
		 store	e1,8(a1)	;
		 lea	16(a1),a1	;
		;6cyc/4 Pixels = 1.5cyc/pixel

	CONVMID4P		;end of regular xloop

		movex.l	(a0)+,d4
		move.l	d4,(a1)+

	CONVEND4P		;end of leftover xloop
	CONVLINEEND		;y loop: advance a0/a1, loop if lines left

	movem.l	(sp)+,d4-d5
	rts


	ifne	DOFUSE68

m68kARGBtoBGRA:
	move.l	d6,-(sp)
	CONVHEAD4P		;set up d5 (trash D0,D4 but D4 is useable now)

	rept	2
		move.l	(a0)+,d4
		 move.l	(a0)+,d6
		 ror.w	#8,d4
		swap	d6
		swap	d4
		 ror.w	#8,d6
		 ror.w	#8,d4
		swap	d6
		swap	d4
		 move.l	d4,(a1)+
		move.l	d6,(a1)+
		;7cyc/2 Pixels = 3.5cyc/pixel
	endr

	CONVMID4P		;end of regular xloop

		move.l	(a0)+,d4
		ror.w	#8,d4
		swap	d4
		ror.w	#8,d4
		swap	d4
		move.l	d4,(a0)+

	CONVEND4P		;end of leftover xloop
	CONVLINEEND		;y loop: advance a0/a1, loop if lines left

	move.l	(sp)+,d6
	movem.l	(sp)+,d4-d5
	rts

	endc	;DOFUSE68



	ifne	1


	else


	endc





;********************************************************************************************
;*
;* OLD STUFF, LARGELY UNTESTED, NEEDS CONVERSION TO MACRO STYLE
;*
;********************************************************************************************




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
