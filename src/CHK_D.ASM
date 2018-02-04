Ideal

P386
Model use16 large
SmallStack
Locals

Masm
	include     windows.inc
	include     int31.inc
	;includelib  import.lib
Ideal


  WM_START_AVIR  EQU  (WM_USER + 101)

  PUBLIC _hWndChk, _MyCallBack

  EXTRN PASCAL POSTMESSAGE: FAR
  EXTRN hWnd: WORD

CodeSeg

PROC  _hWndChk
__hWndChk  DW  0
ENDP  _hWndChk


PROC  _MyCallBack FAR
;==========================================================
; On entry to this routine:
;
;   DS:SI -> Real mode SS:SP
;   ES:DI -> Real mode call structure
;   Interrupts disabled
;
; On exit:
;   ES:DI -> Real mode call structure
;==========================================================

  Pusha
  Push es ds

  Call POSTMESSAGE PASCAL, [cs:__hWndChk], WM_START_AVIR,\
									0,\
									[es:(Real_Mode_Call_Struc di).RealMode_EDX]

  Pop ds es
  Popa

  cld
  lodsw                              ; Get real mode IP from stack
  mov      [WORD PTR es:di+2Ah], ax  ; Store IP in call struc
  lodsw                              ; Get real mode CS from stack
  mov      [WORD PTR es:di+2Ch], ax  ; Store CS in call struc
  add      [WORD PTR es:di+2Eh], 4   ; Bump real mode SP past CS:IP
  iret                               ; Exit My_Call_Back

ENDP  _MyCallBack

  END

