gcc2_compiled.:
___gnu_compiled_c:
.text
	.even
.globl _mygeta4
.globl _WLUT

_mygeta4:
	movel a6@(40:W),a4
	lea a4@(_SysBase:W),a5
	rts

_WLUT:
	moveml #0x3f36,sp@-
	movel sp@(48),a5
	movel sp@(52),a3
	movel sp@(56),d2
	movel sp@(64),a2
	movel sp@(68),d3
	movel sp@(72),d4
	movel sp@(76),d5
	movel sp@(80),d6
	movel sp@(84),d7
	movel a4@(_CyberGfxBase:W),a6
	movel sp@(44),a0
	movew a5,d0
	movew a3,d1
	movel sp@(60),a1
#APP
	jsr a6@(-0xc6:W)
#NO_APP
	moveml sp@+,#0x6cfc
	rts
