Ideal

;include 'tsrid2.inc'

P386
Model use16 large
SmallStack
Locals

;Group  MyCode  Code2


;Segment  Code2  word public 'Code'
;Assume  cs:MyCode, ds:MyCode
CodeSeg

        PUBLIC  _AllocateUMB, _FreeUMB

oldLink       DB  0
oldStrategy   DW  0


PROC  UMBLink near

        Pushf
        Push    ax bx

        Mov     ax, 5802h
        Int     21h
        Mov     [cs:oldLink], al

        Mov     ax, 5800h
        Int     21h
        Mov     [cs:oldStrategy], ax

        Mov     ax, 5803h
        Mov     bx, 1
        Int     21h

        Mov     ax, 5801h
        Mov     bx, 82h
        Int     21h

        Pop     bx ax
        Popf
        Ret

ENDP UMBLink


PROC  UMBUnLink near

        Pushf
        Push    ax bx

        Mov     ax, 5803h
        Xor     bx, bx
        Mov     bl, [cs:oldLink]
        Int     21h

        Mov     ax, 5801h
        Mov     bx, [cs:oldStrategy]
        Int     21h

        Pop     bx ax
        Popf
        Ret

ENDP UMBUnLink


PROC  _AllocateUMB FAR C  ;INPUT:  bx - size in para
                        ;OUT:    ax - segment if carry=0

        Call    UMBLink

        Push    bx
        Mov     ah, 48h
        Int     21h
        Jnc     @@ex

        Mov     ax, 5801h
        Xor     bx, bx
        Int     21h
        Pop     bx
        Mov     ah, 48h
        Int     21h

        Call    UMBUnLink
        Ret

@@ex:
        Call    UMBUnLink
        Add     sp, 2
        Ret

ENDP _AllocateUMB


PROC  _FreeUMB FAR C   ;INPUT: es - segment

        Call    UMBLink
        Push    ax
        Mov     ah, 49h
        Int     21h

        Pop     ax
        Call    UMBUnLink
        Ret

ENDP _FreeUMB

;ENDS Code2

        END
