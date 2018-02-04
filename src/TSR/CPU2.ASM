Ideal

Model tiny
SmallStack
Locals


Group  MyCode  Code3

Segment  Code3  word public 'Code'
Assume  cs:MyCode

        PUBLIC GetCPU

PROC GetCPU  ;result in AX
                Push    dx

                Mov     dx, 0
                Cli
                Pushf

                Pushf
                Pop     ax
                And     ax, 0FFFh
                Or      ax, 5000h
                Push    ax
                Popf
                Pushf
                Pop     ax

                Popf
                Sti
                And     ax, 0F000h
                Cmp     ax, 0F000h
                Jne     @@L_286
                Jmp     @@done

P286
@@L_286:
                Mov     dx, 1
                Or      ax, ax
                Jne     @@L_386
                Jmp     @@done

P386
@@L_386:
                 Mov   dx, 2
                 Cli

                 Pushfd

                 Pushfd
                 Pop    eax
                 Mov    ebx, eax
                 Xor    eax, 40000h
                 Push   eax
                 Popfd
                 Pushfd
                 Pop     eax

                 Popfd
                 Sti
                 And     eax, 40000h
                 And     ebx, 40000h
                 Cmp     eax, ebx
                 Je      @@done

P486
                 Mov     dx, 3

                 Cli
                 Pushfd

                 Pushfd
                 Pop      eax
                 Mov      ebx, eax
                 Xor      eax, 200000h
                 Push     eax

                 Popfd
                 Pushfd
                 Pop      eax

                 Popfd
                 Sti
                 And      eax, 200000h
                 And      ebx, 200000h
                 Cmp      eax, ebx
                 Je       @@done

P586
                 Mov      eax, 1
                 CPUID
                 And      eax, 0F00h
                 Shr      eax, 8
                 Mov      dx, 4
                 Cmp      eax, 5
                 Je       @@done

                 Add      al, 30h
                 Mov      dx, 5

@@done:
                 Mov    ax, dx
                 Pop    dx
                 Ret

ENDP GetCPU

ENDS  Code3

        END

