; ------------------------------------------------------------------------------
; | Apollo AMMX enable                                                         |
; | Henryk Richter <henryk.richter@gmx.net>                                    |
; | All rights reserved                                                        |
; ------------------------------------------------------------------------------
;
;

	XDEF	_Apollo_EnableAMMX
	XDEF	_Apollo_AMMXon		;0.b = Off, 1.b = On
	XDEF	_Apollo_AMMX2on		;0.b = Off, 1.b = On (BSEL, PCMP)


	include	exec/types.i
	include	lvo/exec_lib.i
	include	exec/exec.i

	section	code,code

; ASM _SAVEDS int Apollo_EnableAMMX( void );
;
; return: 0 = no AMMX available
;         1 = AMMX ready for use

_Apollo_EnableAMMX:
		movem.l	d1-a6,-(sp)
		sf	_Apollo_AMMX2on	

		suba.l	a1,a1
		move.l	4.w,a6
		jsr	_LVOFindTask(a6)
		move.l	d0,a4

		;move.l	4.w,a6
		jsr	_LVODisable(A6)
		move.l	TC_TRAPCODE(a4),-(sp)

			;test if we are running on Apollo
			move.l	#Apollo_TrapCatch,TC_TRAPCODE(a4)
			clr.b	TrapCaught

			;AMMX test: perm D0 to D0 and see if we get the desired byte swap
			move.l	#$DEADBEEF,d0
			dc.w	$FE3F,$0000,$807C,$5476	;VPERM   #$807C5476,D0,D0,D0 (actually embeds or.w #$5476,d0 when interpreted as 2 word op)
			nop				;just in case the trap is triggered somewhere else 

			cmp.l	#$ADDEEFBE,d0
			sne	TrapCaught			;behave as if instruction was not executed in case we get the wrong result

		move.l	(sp)+,TC_TRAPCODE(a4)
		move.l	4.w,a6
		jsr	_LVOEnable(A6)


		moveq	#0,d0			;return: fail
		tst.b	TrapCaught
		bne.s	.noAMMX

		; 2nd test for AMMX2: check 
		jsr	_LVODisable(A6)
		move.l	TC_TRAPCODE(a4),-(sp)

		move.l	#Apollo_TrapCatch,TC_TRAPCODE(a4)
		move.l	#$DEADBEEF,D0
		move.l	#$FF00FF00,D1
		move.l	#$55445544,D2
		dc.w	$FE00,$1229	;BSEL D0,D1,D2 = $FE00,$1229 (if(D1&1) D0 else D2 )
		nop
		nop			;see below, addq.l #8
		nop
		cmp.l	#$DE44BE44,d2
		sne	TrapCaught

		move.l	(sp)+,TC_TRAPCODE(a4)
		move.l	4.w,a6
		jsr	_LVOEnable(A6)

		ifne	0

		 moveq	#1,d0		;def: on
		 add.b	TrapCaught,d0	;if( -1 ) d0 = 0  else  d0 = 1

		else

		 moveq	#0,d0
		 tst.b	TrapCaught
		 bne.s	.noAMMX2
		 moveq	#1,d0
.noAMMX2:
		endc

		move.b	d0,_Apollo_AMMX2on	
.enableAMMX:
		; set Bit #11 in SR to announce the use of AMMX
		move.l	4.w,a6
		jsr	_LVODisable(a6)
		jsr	_LVOSuperState(a6)
		move	sr,d1
		or.w	#$800,d1
		move	d1,sr
		jsr	_LVOUserState(a6)
		jsr	_LVOEnable(a6)
		moveq	#1,d0			;return: ok

.noAMMX
		move.b	d0,_Apollo_AMMXon	
		movem.l	(sp)+,d1-a6
		rts

;**********************************************************
; Check whether an Apollo instruction causes a Trap
; assumes 10 byte of code (2 + 8)
;**********************************************************
Apollo_TrapCatch:
		st	TrapCaught
		ADDQ    #4,SP
		ADDQ.L	#8,2(sp)	;VPERM  #....,....,...
		rte

	section	data,bss

TrapCaught:	 ds.b	1
_Apollo_AMMXon:	 ds.b	1
_Apollo_AMMX2on: ds.b	1
		 ds.b	1	;alignment




