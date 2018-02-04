Ideal

P386
Model use16 large
SmallStack
Locals


  PUBLIC _RealIntSimulate

CodeSeg

PROC _RealIntSimulate C
			 ARG pInfo:DWORD, iBX:WORD, iCX:WORD
			 USES es, di, cx, bx

			 Mov     ax, 300h
			 Mov     bx, [iBX]
			 Mov     cx, [iCX]
			 Xor     edi, edi
			 Les     di, [pInfo]
			 Int     31h
			 Jnc     @@ok

			 Mov     ax, -1
			 Ret

@@ok:
			 Xor     ax, ax
			 Ret

ENDP _RealIntSimulate


  END
