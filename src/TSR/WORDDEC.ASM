.Model tiny, c

Group  MyCode  Code3

        PUBLIC  c WordDec

Segment  Code3  word public 'Code'
Assume  cs:MyCode

;___________________  @Word_$Dec  ___________________
;
;   This procedure make convert unsign word to decimal string of ASCII code
;
;   INPUT:    ARGbyte   - byte with sign to convert
;             ARGstrOut - string pointer for out
;
;   OUT:      register AX - len of converted string
;             [ARGstrOut] - decimal string closed 0

WordDec PROC c uses bx cx dx di es, ARGbyte:WORD, ARGstrOut:DWORD

     Xor cx,cx
     Mov ax,ARGbyte
     Xor dx,dx
     Les di,ARGstrOut  ; es:di - out str.
     Mov bx,10

next_div:
     Div bx
     Push dx
     Xor dx,dx
     Inc cx
     Or ax,ax
     Jnz next_div

     Mov dx,cx
     Cld
put_dec_str:
     Pop bx
     Mov al,cs:digits[bx]
     Stosb
     Loop put_dec_str

     Mov BYTE PTR es:[di],'$'

     Mov ax,dx
     Ret

digits DB '0123456789'

WordDec ENDP

ENDS  Code3

        END
