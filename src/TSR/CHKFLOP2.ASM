Ideal

include 'dossys.ash'
include 'tsrid2.inc'
include 'avir.inc'
Masm
  include int2fapi.inc
Ideal

P386
Model use16 tiny
SmallStack
Locals

            REQVEST_PROCESSOR    EQU  2 ;80386
            NUMB_DRIVERS         EQU     26

            ASK_CODE             EQU  00FF06h   ;код запроса для проверки присутствия программы в памяти
            REGISTER_CALLBACK    EQU  00FF05h   ;bx - Real mode callback seg; cx - off
            UNREGISTER_CALLBACK  EQU  00FF04h
            END_TEST             EQU  00FF03h   ;
            GET_STA_DIR          EQU  00FF02h   ;OUT: es:bx - ptr to path
            GET_CONFIG           EQU  00FF01h   ;OUT: ebx - paramBlc
            INI_LOAD             EQU  00FF00h   ;IN: es:bx - paramBlc
            _u_         EQU       16h

            ERR_RATE    EQU      800   ;параметры звукового сигнала ошибки
            ERR_DEL     EQU      200   ;

            STK_SIZE    EQU      1024*2  ;bytes - размер собственного статического стека
            MEM_SIZE    EQU      3520    ;para = 200kB 12800 - размер резервируемого при инициализации блока памяти для запуска антивируса
                                         ;300kB 19200

            SERIAL_SIZE_MAX EQU      25

typedef PSP_PTR far ptr PSP ;указатель на префикс программного сегмента


STRUC FCB

     f DB 0
       DB 11 Dup(?)
       DB 25 Dup(0)

  ENDS


Group  MyCode  Code0, Code1, Code2, Code3, Code4, StkSeg
; Программа сделана в модели tiny и поэтому состоит всего из одного сегмента
; кода. Сегменты компануются в один в порядке их описания в файле chkflop.asm.
; Чтобы внутри каждого сегмента использовать смещения от начала получившегося
; единого сегмента, они объединены в группу. Для первого сегмента задано смещение
; его начала Org 100h - это оставлено место для PSP. Когда программа становится
; резидентной, то в памяти остётся часть кода от начала файла до метки install
; - сегменты Code0, Code2 и StkSeg. Остальные сегменты содержат инициализирующий
; код и отбрасываются, так как описаны после метки install.

        EXTRN  GetCPU         :PROC
        EXTRN  TestVirus      :PROC
        EXTRN  Beep           :PROC
        EXTRN  TimerInit      :PROC
        EXTRN  Play           :PROC
        EXTRN  KeepDisplay    :PROC
        EXTRN  RestoreDisplay :PROC
        EXTRN  _WordDec       :PROC

        PUBLIC  File_length, File_CRC  ;длина и контрольная сумма файла
        PUBLIC  start, end_start       ;метки начала и конца кода

Segment  Code0  para public 'Code'
Assume   cs:MyCode, ds:MyCode

        Org 100h

start:
StartUpCode

        Jmp     install

enblStart       DB              0      ;семафор обработчика Int9 - используется для блокировки от повторного вхождения в собственный обработчик
enblGo          DB              0      ;флаг завершения инициализации резидента - устанавливается в 1 после её завершения и разрешает работу собственных обработчиков прерываний

mountFlag       DB 0                   ;флаг замены диска
mountWin        DB 0                   ;флаг замены диска
avirActive      DB 0                   ;семафор блокирующий работу собственных обработчиков прерывания во время работы антивируса
dosActive       DD ?                   ;адрес флага активности DOS

callBack        DD 0
flInWindows     DB 0

paramBlc        DD 0
;bFlCheckSerial DB 1
;avirusL


;bootSek    DB 1024 Dup(0)
dVectorTblSta   DB 1024 Dup(0)
LABEL  bootSek  BYTE
dVectorTblKeep  DB 1024 Dup(0)

tsrInfo        TSR_Info_Struc { TSR_Exec_Flags = TSR_WINEXEC,\
                                TSR_Exec_Cmd_Show = 4\
                              }
strWExec      DB 256 Dup(0)
STA01  =  $
tsrID         DW   (STA02 - STA01)
STA00  =  $
pass      DB  "CheckFlop 2.0a"
LEN00  =  $-STA00
STA02  =  $
          DB 0

memSize   DW  MEM_SIZE

serialList DD SERIAL_SIZE_MAX+1  Dup(0)
sHead      DW OFFSET cs:serialList
sTail      DW OFFSET cs:serialList
SER_HEAD  EQU  OFFSET cs:serialList
SER_TAIL  EQU  (SER_HEAD) + (SERIAL_SIZE_MAX-1) * 4
ser_tailW  DW ?

int_mask DB  ?
DisableInterrupts:
        Push    ax

        Mov     al, 8fh                 ; немаскируемые прерывания запрещаются
        Out     70h, al                 ; битом 80h при доступе к CMOS
        Jmp     $+2                     ; небольшая задержка
        In      al, 70h+1

        In      al, 021h                ; разрешение маскируемых прерываний
        Mov     [cs:int_mask], al       ; запомним маску прерываний
        Mov     al, 0ffh                ; запретим прерывания контроллеру
        Out     021h, al                ; прерываний

        Cli
        Pop     ax
        Retn

EnableInterrupts:
        Push    ax

        Mov     ax, 00fh                ; разрешение немаскируемых прерываний
        Out     70h, al
        Jmp     $+2                     ; небольшая задержка
        In      al, 70h+1

        Mov     al, [cs:int_mask]       ; восстановим маску прерываний
        Out     21h, al                 ; разрешим маскируемые прерывания

        Sti
        Pop     ax
        Retn

PROC  GetVTable near ;IN: es:di - keep array 1024 bytes
        Push  si ds cx

        Call  DisableInterrupts
        Xor   si, si
        Mov   ds, si
        Mov   cx, 256
        Cld
        REP   Movsd
        Call  EnableInterrupts

        Pop   cx ds si
        Ret
ENDP  GetVTable

PROC  SetVTable near ;IN: ds:si - source array 1024 bytes

        Push  di es cx

        Call  DisableInterrupts
        Xor   di, di
        Mov   es, di
        Mov   cx, 256
        Cld
        REP   Movsd
        Call  EnableInterrupts

        Pop   cx es di
        Ret
ENDP  SetVTable


PROC  AddSerisl near

        Push   bx cx

        Mov    bx, [cs:sTail]
        Mov    [cs:bx], eax
        Add    bx, 4
        Cmp    bx, [cs:ser_tailW]
        Ja     @@over
@@cmpH:
        Cmp    bx, [cs:sHead]
        Jz     @@erase
@@store:
        Mov    [cs:sTail], bx

        Pop    cx bx
        Ret
@@over:
        Mov    bx, SER_HEAD
        Jmp    SHORT @@cmpH

@@erase:
        Mov    cx, [cs:sHead]
        Add    cx, 4
        Cmp    cx, [cs:ser_tailW]
        Ja     @@over2
@@cmpH2:
        Mov    [cs:sHead], cx
        Jmp    SHORT  @@store
@@over2:
        Mov    cx, SER_HEAD
        Jmp    SHORT @@cmpH2

ENDP  AddSerisl

PROC  FindSerial ;IN: eax=serial; OUT: carry=1 if not found
        PushAllE
        Push   gs

        Push   cs cs
        Pop    es ds

        Lgs    si, [paramBlc]
        Movzx  ecx, [word ptr sTail]
        Cmp    cx, [sHead]
        Ja     @@1_
        Jz     @@notf
        Xor    edi, edi
        Mov    di, OFFSET cs:serialList
        ;;;;;;Mov    cx, SERIAL_SIZE * 4
        Mov    cx, [gs:(TParamBlock si).opt.iSizeOfSerialQueue]
        Shl    cx, 2
        Jmp    SHORT @@scs
@@1_:
        Movzx  edi, [word ptr sHead]
        Sub    cx,  [sHead]
        Add    cx, 4

@@scs:
        Shr    cx, 2
        Cld
        REPNZ  Scasd
        Jz     @@ok
@@notf:
        Stc
        Jmp    SHORT @@end
@@ok:
        Clc
@@end:
        Pop    gs
        PopAllE
        Ret
ENDP  FindSerial

PROC  CheckSerial near; IN: dl = 0(A), 1(B)..; OUT: carry

        PushAllE

        Lds    bx, [cs:paramBlc]
        Cmp    [(TParamBlock bx).opt.flCheckSerial], 0
        Jz     @@needTest

        Sub    dl, 'A'
        Call   LoadBootSek
        Jc     @@notTest
        Cmp    [byte ptr cs:bootSek + 26h], ')'
        Jnz    @@needTest
        Mov    eax, [dword ptr cs:bootSek + 27h]
        Call   FindSerial
        Jnc    @@notTest
        Call   AddSerisl

@@needTest:
        PopAllE
@@testAll:
        Stc
        Ret

@@notTest:
        PopAllE
        Clc
        Ret

ENDP  CheckSerial

PROC  LoadBootSek near ;IN: dl = 0(A), 1(B)..

        PushAll

        Mov    cx, 01h
        Xor    dh, dh
        Mov    ax, cs
        Mov    ds, ax
        Mov    bx, OFFSET cs:bootSek
        Xor    si, si
        Les    di, [paramBlc]
        Cmp    [flInWindows], 0
        Jz     @@DOS
        Mov    di, [es:(TParamBlock di).opt.iRetryNumberSec1ReadWin]
        Jmp    SHORT @@ent1
@@DOS:
        Mov    di, [es:(TParamBlock di).opt.iRetryNumberSec1ReadDOS]

@@ent1:
        Mov    es, ax
@@retry:
        Mov    ax, 0201h
        Pushf
        Call   [Old13h]
        Jnc    @@ok
        Inc    si
        Cmp    si, di
        Jae     @@err
        Jmp    SHORT @@retry

@@err:
        Stc
@@ok:
        PopAll
        Ret

ENDP  LoadBootSek

PROC  StrLen ;cs:di

        Cmp    [byte ptr cs:di], 0
        Jnz    @@enter
        Xor    cx, cx
        Ret

@@enter:
        Push   es ax di

        Mov    cx, 0FFh
        Mov    ax, cs
        Mov    es, ax
        Xor    al, al
        Cld
        REPNZ  Scasb

        Sub    cx, 0FFh
        Neg    cx
        Dec    cx

        Pop    di ax es
        Ret

ENDP  StrLen

PROC  Prepare_strWExec near ;IN: di - offs. out str; si - offs. name, cx - len name

        PushAll

        Push   si cx

        Call   GetStaDIR
        Push   es
        Pop    ds
        Mov    si, bx
        Push   cs
        Pop    es
        ;Mov    di, OFFSET cs:strWExec
        Mov    cx, 128
        Cld
@@loop:
        Lodsb
        Stosb
        Or    al, al
        Jz    @@Ok
        Loop  @@loop

@@Ok:
        Mov   al, '\'
        Mov   cx, 128
        Std
        REPNZ Scasb
        Add   di, 2
        Push  cs
        Pop   ds
        Pop   cx si
        ;Mov   si, OFFSET cs:winName
        ;Mov   cx, LEN02
        Cld
        REP   Movsb
        Dec   di
        Mov   [byte ptr es:di], 0
        Mov   [byte ptr es:di + 1], '$'

        PopAll
        Ret

ENDP  Prepare_strWExec

PROC  GetStaDIR near ;OUT: es:bx

        Push    ax cx di

        Mov     ax, [cs:2Ch]
        Mov     es, ax
        Xor     di, di
        Xor     ax, ax
        Mov     cx, 0FFFEh
@@enter:
        Cld
        REPNZ   Scasb
        Cmp     [byte ptr es:di], 0
        Jnz     @@enter

        Mov     bx, di
        Add     bx, 3

        Pop     di cx ax
        Ret

ENDP  GetStaDIR

PROC  New2Fh  far                ;Обработчик мультиплексорного прерывания
                                 ;служит для проверки наличия этой программы
                                 ;в памяти с целью исключения повторной загрузки.

        Cmp     ax, ASK_CODE     ;При инициализации вызывается Int2F с условным
        Jnz     @@next1          ;кодом в AX, и если программа уже инсталирована,
                                 ;то в DS:SI возвращается адрес ключевой строки.
        Mov     si, OFFSET cs:pass
        Mov     ax, cs
        Mov     ds, ax
        Mov     ax, ASK_CODE
        Jmp     @@termOK

@@next1:
        Cmp     ax, REGISTER_CALLBACK ;bx - Real mode callback seg; cx - off
        Jnz     @@next2

        Mov     [word ptr cs:callBack], cx
        Mov     [word ptr cs:callBack+2], bx
        Jmp     @@termOK

@@next2:
        Cmp     ax, UNREGISTER_CALLBACK
        Jnz     @@next3

        Mov     [cs:callBack], 0
        Mov     [cs:avirActive], 0
        Jmp     @@termOK

@@next3:
        Cmp     ax, END_TEST
        Jnz     @@next4
        Mov     [cs:avirActive], 0
        Jmp     @@termOK

@@next4:
        Cmp     ax, GET_STA_DIR
        Jnz     @@next5
        Call    GetStaDIR       ;OUT: es:bx
        Jmp     @@termOK

@@next5:
        Cmp     ax, GET_CONFIG ;OUT: ebx - paramBlc
        Jnz     @@next6
        Mov     ebx, [cs:paramBlc]
        Jmp     @@termOK

@@next6:
        Cmp     ax, 1605h
        Jnz     @@next8
        Sti
        Pushf
        Call    [cs:Old2Fh]
        Or      cx, cx
        Jnz     @@iret
        Test    dx, 1b
        Jz      @@OkLoad
        Mov     cx, 1
        Jmp     @@iret
@@OkLoad:
        Mov     [cs:flInWindows], 1
        Jmp     @@iret

@@next8:
       Cmp     ax, 1606h
       Jnz     @@next9

       Mov     [cs:flInWindows], 0
       Jmp     SHORT @@iret

@@next9:
       Cmp     ax, 160Bh
       Jnz     @@next10
       Sti
       Push    bx
       Mov     bx, OFFSET cs:tsrInfo
       Mov     [cs:(TSR_Info_Struc bx).TSR_PSP_Segment], cs
       Mov     [word ptr cs:(TSR_Info_Struc bx).TSR_Next], di
       Mov     [word ptr cs:(TSR_Info_Struc bx+2).TSR_Next], es
       Mov     [word ptr cs:(TSR_Info_Struc bx+2).TSR_ID_Block], cs
       Mov     [word ptr cs:(TSR_Info_Struc bx).TSR_ID_Block], OFFSET cs:tsrID
       Mov     [word ptr cs:(TSR_Info_Struc bx+2).TSR_Exec_Cmd], cs
       Mov     [word ptr cs:(TSR_Info_Struc bx).TSR_Exec_Cmd], OFFSET cs:strWExec


       Push    si di cx
       Mov     di, OFFSET  cs:strWExec
       Mov     si, OFFSET  cs:@@winName
       Mov     cx, LEN02
       Call    Prepare_strWExec ;IN: di - offs. out str; si - offs. name, cx - len name
       Pop     cx di si

       Mov     di, bx
       Push    cs
       Pop     es
       Pop     bx

@@next10:
       Cmp     ax, INI_LOAD ;es:bx - paramBlc
       Jnz     @@exit_

       Mov     [word ptr cs:paramBlc + 2], es
       Mov     [word ptr cs:paramBlc], bx
       Push    di
       Mov     di, [es:(TParamBlock bx).avL]
       Mov     di, [es:(TAVirusList di).ucMemMax]
       Mov     [cs:memSize], di
       Mov     di, [es:(TParamBlock bx).opt.iSizeOfSerialQueue]
       Dec     di
       Shl     di, 2
       Add     di, SER_HEAD
       Mov     [cs:ser_tailW], di
       Pop     di
       Jmp     SHORT @@termOK

@@exit_:
        Jmp     [cs:Old2Fh]      ;переход на старый обработчик Int2F

@@termOK:
        Xchg    al, ah
@@iret:
        Iret

Old2Fh    DD  ?                  ;адрес старого обработчика

STA05 = $
@@winName  DB  'CHK_F.EXE', 0
LEN02 = $ - STA05

ENDP New2Fh

PROC  New25h                      ;Прямое чтение сектора используется для запуска
                                  ;антивируса в случае обращения к заменённому диску.
        ;Cmp     [cs:enblGo], 0    ;После вызова старого обработчика анализируется флаг замены диска
        ;Jz      @@leave           ;устанавливаемый в обработчике Int13h и если он 1 и DOS не активна, то запускается антивирус.
        ;Cmp     [cs:avirActive], 0
        ;Jz      @@enter
@@leave:
        Jmp     [cs:Old25h]       ;переход на старый обработчик

@@enter:
        Pushf
        Call    [cs:Old25h]       ;вызов старого обработчика (Pushf не надо - Int25h не извлекает флаги из стека).

        Pushf
        Cmp     [cs:mountFlag], 0 ;проверка смены диска
        Jz      @@ex

        PushAll
        Lds     bx, [cs:dosActive] ;проверка активности DOS
        Cmp     [byte ptr bx], 0
        Jnz     @@dosAct

        ;Mov     [cs:avirActive], 1 ;блокировка повторного вхождения в собственные обработчики
        Pushf
        Call    ExecAntivirus      ;запуск антивируса
        Popf
        Mov     [cs:mountFlag], 0
        ;Mov     [cs:avirActive], 0
@@dosAct:
        PopAll

@@ex:
        Popf
        Ret     2                  ;возврат из прерывания без изменения флагов установленных старым обработчиком

Old25h    DD  ?

ENDP New25h


PROC  New9h far                      ;Проверяет нажатие Ctrl+Alt+U и если комбинация нажата выгружает программу

        Cmp     [cs:avirActive], 0
        Jnz     @@leave
        Cmp     [cs:enblStart], 0
        Jz      @@leave
        Cmp     [cs:flInWindows], 0
        Jnz     @@leave

        Push    es ax
        Mov     [cs:enblStart], 0

        Xor     ax, ax
        Mov     es, ax
        Mov     al, [es:0417h]     ;Чтение байта состояния клавиатуры из BIOS
        And     al, 00001111b      ; Ctrl+Alt
        Cmp     al, 00001100b
        Jnz     @@old_inter_pop

        In      al, 60h            ;чтение скан-кода нажатой клавиши
        Cmp     al, _u_
        Jz      @@unload


@@old_inter_pop:
        Pop     ax es

@@old_inter:
        Mov     [cs:enblStart], 1
@@leave:
        Jmp     [cs:Old9h]        ;переход на след. обработчик

@@return:
        Pop     ax es
        Mov     [cs:enblStart], 1
        Iret


@@unload:
        Call    near ptr @@inter_end ;если Ctrl+Alt+U, то аппаратное прерывание от клавиатуры обрабатывается без вызова старого обработчика, что делает это нажатие клавиши незаметным для других программ
        PushAll
        Call    Unload               ;выгрузка программы
        PopAll
        Jmp     SHORT @@return


@@inter_end:
        Push    ax
        Cli
        In      al, 61h       ;разблакировка клавиатуры и завершение обработки прерывания в контроллере прерываний
        Mov     ah, al
        Or      al, 80h
        Out     61h, al
        Xchg    ah, al
        Out     61h, al
        Mov     al, 20h
        Out     20h, al
        Sti
        Pop     ax
        Retn

Old9h     DD  ?

ENDP  New9h

PROC  CheckVectors near
        PushAllE
        Pushf

        Mov     bx, es
        Dec     bx
        Mov     ds, bx
        Xor     bx, bx
        Xor     ecx, ecx
        Mov     cx, [(MCB bx).blockSize]
        Shl     ecx, 4

        Mov     bx, es
        Movzx   esi, bx
        Shl     esi, 4
        Mov     edi, esi
        Add     edi, ecx

        Mov     cx, 256
        Mov     bx, OFFSET cs:dVectorTblSta
        Push    cs
        Pop     ds
        Xor     dx, dx
        Mov     es, dx
@@vLoop:
        Xor     eax, eax
        Mov     ax, [bx + 2]
        Shl     eax, 4
        Add     ax, [bx]
        Cmp     esi, eax
        Ja      @@next
        Cmp     edi, eax
        Jb      @@next

        Push    bx
        Mov     bx, dx
        Mov     eax, [es:bx]
        Pop     bx
        Mov     [bx], eax
                                  

@@next:
        Add     bx, 4
        Add     dx, 4
        Loop    @@vLoop

        Popf
        PopAllE
        Ret
ENDP  CheckVectors

PROC  New21h far                    ;Используется для запуска антивируса в случае замены дмска.
                                    ;Флаг замены проверяется после вызова старого обработчика и
        Cmp     [cs:enblGo], 0      ;если надо запускается антивирус так как DOS находится в реентерабельном состоянии.
        Jz      @@leave             ;
        Cmp     ah, 49h
        Jz      @@checkVectors
@@retVectors:
        Cmp     [cs:avirActive], 0  ;
        Jz      @@enter             ;
@@leave:
        Jmp     [cs:Old21h]

@@enter:
        Pushf
        Call    [cs:Old21h]        ;вызов старого обработчика (Pushf нужен для эммуляции Int21h)

        Pushf
        Cmp     [cs:mountFlag], 0  ;проверка смены диска
        Jz      @@ex

        PushAll
        Pushf
        Call    ExecAntivirus      ;запуск антивируса
        Popf
        Mov     [cs:mountFlag], 0
        PopAll

@@ex:
        Popf
        Ret     2                  ;возврат из прерывания без изменения флагов

@@checkVectors:
        Call    CheckVectors
        Jmp     SHORT @@retVectors

Old21h        DD  ?

ENDP New21h


PROC  New13h  far                  ;Используется для проверки смены диска - dl всегда содержит адрес устройства, где 0, 1.. - сменные, а 80h, 81h.. - несменные.

        Cmp     [cs:enblGo], 0
        Jz      @@leave
        Cmp     [cs:avirActive], 0
        Jz      @@enter
@@leave:
        Jmp     [cs:Old13h]

@@enter:
        Cmp     dl, 80h
        Jae     @@ex

        PushAll

        Mov     ah, 16h
        Pushf                            ;нужно для эммуляции Int13h
        Call    [dword ptr cs:Old13h]    ;вызов старого Int13h для проверки смены диска dl
        Cmp     ah, 6
        Jnz     @@noMount
                                    ;Jmp @@noMount

        Add     dl, 'A'                  ;вычисляем букву устройства
        Call    CheckSerial
        Jnc     @@noMount

        Cmp     [cs:flInWindows], 0
        Jnz     @@win
        Mov     [cs:mountFlag], dl
        Jmp     SHORT @@noMount
@@win:
        Mov     [cs:mountWin], dl
        Pushf
        Call    ExecAntivirus
        Popf

@@noMount:
        PopAll
@@ex:
        Jmp     [dword ptr cs:Old13h]

Old13h    DD  ?
;int13Led  DB  0

ENDP New13h

PROC New24h far
        Mov al,3
New23h:
        PushAll
        Mov     di, ERR_RATE
        Mov     bx, ERR_DEL
        Call    Beep
        PopAll

        Iret

Old24h    DD  ?
Old23h    DD  ?

ENDP  New24h


PROC  Call_DLL_Entry near

      Mov      ax, 1683h                 ; Windows Interrupt 2Fh
      Int      2Fh                       ;    (Get Current VM)
      Cmp      bx, 1                     ; Windows is always VM 1                                         ; (version 3.0 *ONLY*)
      Jz       @@call_DLL                  ; We're in Windows VM

;@@switch_VMs_And_Call_DLL:              ; We're not in Windows VM
      Mov      ax, 1685h                 ; Windows Interrupt 2Fh
                                         ;   switch VMs and callback
      Mov      bx, 1                     ; Windows 3.0 is always VM 1
      Mov      cx, 0                     ; All flags off
      Mov      dx, 0                     ; Priority boost not required
      Mov      si, 0                     ;
      Mov      di, cs                    ; Segment of switch procedure
      Mov      es, di                    ;
      Mov      di, OFFSET cs:SwitchProc  ; Offset of switch procedure
      Int      2Fh                       ;
      Jnc      @@my_1685h_Success

@@my_1685h_Error:
      ; AX = error code:   1 = Invalid VM ID
      ;                    2 = Invalid priority boost
      ;                    3 = Invalid flags
      Jmp      SHORT @@call_DLL_Exit

@@my_1685h_Success:
      Mov     ax, 0
      Jmp     SHORT @@call_DLL_Exit

@@call_DLL:
      Movzx     edx, [byte ptr cs:mountWin]
      Call    [cs:callBack]               ; FAR CALL to real mode
                                          ;   callback
@@call_DLL_Exit:
      Ret

ENDP Call_DLL_Entry

PROC  SwitchProc far
      PushAll

      Movzx     edx, [byte ptr cs:mountWin]
      Call    [cs:callBack]          ; FAR CALL to real mode
                                     ;   callback
      PopAll
      Iret                               ; Switches back to VM from
                                         ;   which it was called
ENDP  SwitchProc


PROC ExecAntivirus near                ;Освобождает зарезервированный блок памяти, переключается стек на свой, и запускает антивирус функцией Exec.

                                     ;Pusha
                                     ;Mov     di, ERR_RATE
                                     ;Mov     bx, ERR_DEL
                                     ;Call    Beep
                                     ;Popa
        Cmp     [cs:callBack], 0
        Jz      @@DOSExec

        Mov     [cs:avirActive], 1
        PushAll
        Call    Call_DLL_Entry
        PopAll
@@ret1:
        Ret

@@DOSExec:
        Cmp     [cs:flInWindows], 0
        Jnz     @@ret1

@@Ok:
        Mov     [cs:avirActive], 1

        Mov     dl, [cs:mountFlag]
        ;Mov     [cs:@@cmdLine+2], dl   ;Запись устройства в строку параметров.
        Mov     [cs:@@device], dl

        Mov     ax, 5900h                      ;сохранение информации об ошибке
        Xor     bx, bx
        Pushf
        Call    [cs:Old21h]
        Mov     [cs:@@oldExtErr], ax
        Mov     [cs:@@oldExtErr+2], bx
        Mov     [cs:@@oldExtErr+4], cx

        Mov     ax, 6200h                ;сохранение текущего PSP
        Pushf
        Call    [cs:Old21h]
        Mov     [cs:@@oldPSP], bx
        Mov     ax, 5000h              ;переключение на свой PSP
        Mov     bx, cs
        Pushf
        Call    [cs:Old21h]

        Mov     [cs:@@OldSS], ss       ;сохранение стека
        Mov     [cs:@@OldSP], sp
        Mov     ax, cs
        Cli
        Mov     ss, ax
        Mov     sp, OFFSET MyCode:myStk ;переключение на свой стек
        Add     sp, STK_SIZE-16
        Sti

        Mov     ah, 2Fh
        Pushf
        Call    [cs:Old21h]
        Mov     [word ptr cs:@@oldDTA], bx
        Mov     [word ptr cs:@@oldDTA + 2], es
        Push    cs
        Pop     ds
        Mov     dx, 80h
        Mov     ah, 1Ah
        Pushf
        Call    [cs:Old21h]

        Mov     di, cs
        Mov     es, di
        Mov     di, OFFSET cs:dVectorTblKeep
        Call    GetVTable

        Mov     si, cs
        Mov     ds, si
        Mov     si, OFFSET cs:dVectorTblSta
        Call    SetVTable
                                 ;ds:si - source array 1024 bytes

        ReDirectVector2  24h
        ReDirectVector2  23h

        Call    KeepDisplay             ;сохранение экрана

        Call    SwappingAndDiscard
        Jnc     @@OkSwp

        Push    cs
        Pop     ds
        Mov     ah, 9h
        Mov     dx, OFFSET cs:@@msgErrSwap
        Pushf
        Call    [cs:Old21h]
        Mov     di, ERR_RATE
        Mov     bx, ERR_DEL
        Call    Beep
        Xor     ah, ah
        Int     16h
        Jmp     @@memRestore

@@OkSwp:
        ;Mov     ax, 5802h               ;сохранение текущей связи UMB
        ;Pushf
        ;Call    [cs:Old21h]
        ;Mov     [cs:@@oldLink], al
        ;Mov     ax, 5803h               ;установка связи с UMB
        ;Mov     bx, 1
        ;Pushf
        ;Call    [cs:Old21h]
                                 ;!!!!!!!!!!
        Lds     bx, [cs:paramBlc]
        Mov     si, [(TParamBlock bx).drLDOS]
        Mov     cx, [(TDrivesList si).iNumbEntries]
        Jcxz    @@memRestore
        Add     si, OFFSET (TDrivesList).deEntryes

@@execL:
        Mov     bl, [(TDriveEntry si).ucDriveDOS]
        Cmp     bl, [cs:@@device]
        Jnz     @@nextDrv

        Mov     di, [(TDriveEntry si).uiIndex]
        Mov     dx, [(TAVirEntry di).ptrStrCom]

        Push    si di es
        Mov     si, [(TAVirEntry di).ptrStrParam]
        Push    cs
        Pop     es
        Mov     di, OFFSET @@cmdLine
        Cld
@@copyL:
        Lodsb
        Stosb
        Or      al, al
        Jnz     @@copyL

        Mov     di, (OFFSET @@cmdLine) + 1
        Mov     al, '#'
        Push    cx
        Mov     cx, 0FEh
        Cld
        REPNZ   Scasb
        Pop     cx
        Mov     al, [cs:@@device]
        Dec     di
        Mov     [es:di], al
        Pop     es di si

        Call    @@Execute   ;ds:dx

@@nextDrv:
        Add     si, SIZE TDriveEntry
        Loop    @@execL

        Push    cs
        Pop     ds
        Mov     ah, 9h
        Mov     dx, OFFSET cs:@@msgTestOk
        Pushf
        Call    [cs:Old21h]

        Xor     ah, ah
        Int     16h

@@memRestore:
        Call    RestoreDisplay  ;восстановление экрана

        ;Mov     ax, 5803h         ;восстановление состояния связи с UMB
        ;Xor     bx, bx
        ;Mov     bl, [cs:@@oldLink]
        ;Pushf
        ;Call    [cs:Old21h]

        ;Mov     bx, [cs:memSize]

        ;UnRedirectVector2  23h
        ;UnRedirectVector2  24h
        Call    FetchAndRestore

        Mov     si, cs
        Mov     ds, si
        Mov     si, OFFSET cs:dVectorTblKeep
        Call    SetVTable

        Mov     dx, [word ptr cs:@@oldDTA]
        Mov     ax, [word ptr cs:@@oldDTA + 2]
        Mov     ds, ax
        Mov     ah, 1Ah
        Pushf
        Call    [cs:Old21h]

        Cli                         ;восстановление стека
        Mov     ss, [cs:@@OldSS]
        Mov     sp, [cs:@@OldSP]
        Sti

        Mov     ax, 5000h             ;восстановление текущего PSP
        Mov     bx, [cs:@@oldPSP]
        Pushf
        Call    [cs:Old21h]

        Mov     ax, 5D0Ah
        Xor     bx, bx
        Push    cs
        Pop     ds
        Mov     dx, OFFSET MyCode:@@oldExtErr
        Pushf
        Call    [cs:Old21h]

        Mov     [cs:avirActive], 0
        Ret

@@Execute:
                               ;ds:dx
        PushAll

        Push    ds dx
        Push    cs
        Pop     ds
        Mov     ah, 9h
        Mov     dx, OFFSET cs:@@msgStartAV
        Pushf
        Call    [cs:Old21h]
        ;Mov     dx, OFFSET cs:@@strCom1
        Pop     dx ds
        Pushf
        Call    [cs:Old21h]
        Push    ds dx
        Push    cs
        Pop     ds
        Mov     dx, OFFSET cs:@@msgStartAVd
        Pushf
        Call    [cs:Old21h]

        Pop     dx ds

        Mov     [cs:@@OldSS_Exec], ss       ;сохранение стека
        Mov     [cs:@@OldSP_Exec], sp

        Mov     ax, 4B00h               ;DOS Exec
        Push    cs
        Pop     es                ;ds:dx - path
        ;Mov     dx, OFFSET MyCode:@@strCom1
        Mov     bx, OFFSET MyCode:paramsBlc
        Pushf
        Call    [cs:Old21h]
        Lahf

        Mov     bx, [cs:@@OldSS_Exec]                  ;восстановление стека на свой
        Cli
        Mov     ss, bx
        Mov     sp, [cs:@@OldSP_Exec]
        Sti

        Sahf
        Jnc     @@exec_ok
        Push    cs
        Pop     ds
        Mov     ah, 9h
        Mov     dx, OFFSET cs:@@msgErrAVStart
        Pushf
        Call    [cs:Old21h]
        Mov     di, ERR_RATE
        Mov     bx, ERR_DEL
        Call    Beep
        Xor     ah, ah
        Int     16h
        Stc
        Jmp     SHORT @@exec_ret

@@exec_ok:
        Clc

@@exec_ret:
        PopAll
        Retn

@@OldSS   DW ?
@@OldSP   DW ?
@@OldSS_Exec   DW ?
@@OldSP_Exec   DW ?
@@oldPSP  DW ?
;@@oldLink DB ?
@@oldDTA  DD ?

@@oldExtErr DW 3 Dup(?)

;@@strCom1       DB STRPATH_AV1, 0, '$'
;@@cmdLine       DB 15, ' a:\ /q/f/g/s/n', 0Dh
;@@cmdLine       DB 4,' a:\', 0Dh
@@cmdLine         DB 128 Dup(0)

@@msgTestOk     DB 10, 13, 'CHKFLOP2: тестирование завершилось...', 10, 13, '$'
@@msgErrSwap    DB 10,13, 'CHKFLOP2: Ошибка своппинга', 10,13,'$'
@@msgStartAV    DB 10,13, 'Запуск антивируса  $'
@@msgStartAVd   DB '  для устройства '
LABEL @@device BYTE
                DB 'A:', 10,13,10,'$'
@@msgErrAVStart DB 'Ошибка запуска антивируса', 10,13,'$'

@@fcb1    FCB <>
@@fcb2    FCB <>

paramsBlc    ParsTyp <, OFFSET MyCode:@@cmdLine,, OFFSET MyCode:@@fcb1,, OFFSET MyCode:@@fcb2,>


ENDP ExecAntivirus

STA_SWP = $
swpName  DB  'SWAP.SWP', 0
LEN_SWP = $ - STA_SWP


PROC  GetStartFree; OUT: esi
       Push    edx bx

       Mov     bx, cs
       Dec     bx
       Mov     ds, bx
       Xor     bx, bx
       Xor     edx, edx
       Mov     dx, [word ptr (MCB bx).blockSize]
       Shl     dx, 4

       Push    cs
       Xor     esi, esi
       Pop     si
       Shl     esi, 4
       Add     esi, edx

       Pop     bx edx
       Ret
ENDP  GetStartFree

PROC  GetEndFree; OUT: edi
       Xor     edi, edi
       Mov     di, [word ptr cs:paramBlc + 2]
       Dec     di
       Shl     edi, 4

       Ret
ENDP  GetEndFree

PROC  GetSwpAddr ;OUT: ds:dx
       Push    bx

       Mov     bx, cs
       Dec     bx
       Mov     ds, bx
       Xor     bx, bx
       Mov     dx, [word ptr (MCB bx).blockSize]

       Mov     bx, cs
       Add     bx, dx
       Mov     ds, bx
       Xor     dx, dx

       Pop     bx
       Ret
ENDP  GetSwpAddr

PROC  Add0FFF0h ;ds:dx
       Push    ax bx cx

       Mov     ax, dx
       Xor     dx, dx
       Mov     cx, 16
       Div     cx
       Mov     bx, ds
       Add     bx, ax
       Mov     ds, bx

       Add     dx, 0FFF0h
       Mov     ax, dx
       Xor     dx, dx
       Div     cx
       Mov     bx, ds
       Add     bx, ax
       Mov     ds, bx

       Pop     cx bx ax
       Ret
ENDP  Add0FFF0h

mcbMCB MCB <'M',0,0,0>
bFlSwp DB 0

;firstSymb           DB      ?
;ownerPSP            DW      ?
;blockSize           DW      ?
;reserved            DB      11 DUP ( ? )


PROC SwappingAndDiscard near
       PushAllE
       Mov     [cs:bFlSwp], 0

       Push    si di cx
       Mov     di, OFFSET  cs:strWExec
       Mov     si, OFFSET  cs:swpName
       Mov     cx, LEN_SWP
       Call    Prepare_strWExec ;IN: di - offs. out str; si - offs. name, cx - len name
       Pop     cx di si

       Mov     ah, 3Ch
       Xor     cx, cx
       Push    cs
       Pop     ds
       Mov     dx, OFFSET cs:strWExec
       Pushf
       Call    [cs:Old21h]
       Jc      @@E_CanntSwp

       Push    ax ;handle

       Call    GetStartFree ; OUT: esi
       Call    GetEndFree   ; OUT: edi
       Sub     edi, esi
       Mov     eax, edi
       Xor     edx, edx
       Mov     ecx, 0FFF0h
       Div     ecx

       Mov     ecx, eax  ;сколько раз по 0FFFEh
       Mov     esi, edx  ;остаток

       Call    GetSwpAddr ;ds:dx

       Pop     bx ;handle
@@write:
       Mov     ah, 40h
       Push    cx
       Mov     cx, 0FFF0h
       Pushf
       Call    [cs:Old21h]
       Jc      @@E_CanntSwpClosePop
       Cmp     cx, ax
       Pop     cx
       Jnz     @@E_CanntSwpClose
       Call    Add0FFF0h
       Loop    @@write

       Mov     cx, si
       Mov     ah, 40h
       Pushf
       Call    [cs:Old21h]
       Jc      @@E_CanntSwpClose
       Cmp     cx, ax
       Jnz     @@E_CanntSwpClose

       Mov     ah, 3Eh
       Pushf
       Call    [cs:Old21h]
       Jc      @@E_CanntSwp


       Call    GetStartFree ; OUT: esi
       Call    GetEndFree   ; OUT: edi
       Sub     edi, esi
       Sub     edi, 16
       Mov     eax, edi
       Xor     edx, edx
       Mov     ecx, 16
       Div     ecx
       Mov     [cs:mcbMCB.blockSize], ax

       Call    GetSwpAddr ;ds:dx
       Push    ds
       Pop     es
       Push    cs
       Pop     ds
       Mov     si, OFFSET cs:mcbMCB
       Mov     di, dx
       Mov     cx, 4
       Cld
       REP     Movsd

       Jmp     SHORT @@ex

@@E_CanntSwpClosePop:
       Pop     cx
@@E_CanntSwpClose:
       Mov     ah, 3Eh
       Pushf
       Call    [cs:Old21h]
       Mov     ah, 41h
       Push    cs
       Pop     ds
       Mov     dx, OFFSET cs:strWExec
       Pushf
       Call    [cs:Old21h]
@@E_CanntSwp:
       PopAllE
       Stc
       Ret

@@ex:
       PopAllE
       Mov     [cs:bFlSwp], 1
       Clc
       Ret

ENDP SwappingAndDiscard

PROC  FetchAndRestore near
       Cmp     [cs:bFlSwp], 0
       Jz      @@ex
       Mov     [cs:bFlSwp], 0

       PushAllE

       Mov     ax, 3D00h
       Push    cs
       Pop     ds
       Mov     dx, OFFSET cs:strWExec
       Pushf
       Call    [cs:Old21h]
       Jc      @@E_CanntFetch

       Push    ax ;handle

       Call    GetStartFree ; OUT: esi
       Call    GetEndFree   ; OUT: edi
       Sub     edi, esi
       Mov     eax, edi
       Xor     edx, edx
       Mov     ecx, 0FFF0h
       Div     ecx

       Mov     ecx, eax  ;сколько раз по 0FFFEh
       Mov     esi, edx  ;остаток

       Call    GetSwpAddr ;ds:dx

       Pop     bx ;handle
@@read:
       Mov     ah, 3Fh
       Push    cx
       Mov     cx, 0FFF0h
       Pushf
       Call    [cs:Old21h]
       Jc      @@E_CanntSwpClosePop
       Cmp     cx, ax
       Pop     cx
       Jnz     @@E_CanntSwpClose
       Call    Add0FFF0h
       Loop    @@read

       Mov     cx, si
       Mov     ah, 3Fh
       Pushf
       Call    [cs:Old21h]
       Jc      @@E_CanntSwpClose
       Cmp     cx, ax
       Jnz     @@E_CanntSwpClose

       Mov     ah, 3Eh
       Pushf
       Call    [cs:Old21h]
       Jc      @@E_CanntFetch

       Mov     ah, 41h
       Push    cs
       Pop     ds
       Mov     dx, OFFSET cs:strWExec
       Pushf
       Call    [cs:Old21h]

       PopAllE
       Clc
       Ret

@@E_CanntSwpClosePop:
       Pop     cx
@@E_CanntSwpClose:
       Mov     ah, 3Eh
       Pushf
       Call    [cs:Old21h]
@@E_CanntFetch:
       Mov     ah, 41h
       Push    cs
       Pop     ds
       Mov     dx, OFFSET cs:strWExec
       Pushf
       Call    [cs:Old21h]

       PopAllE
       Stc
       Ret
@@ex:
      Clc
      Ret
ENDP  FetchAndRestore


PROC  Unload           ;Восстанавливает вектора прерываний прямой записью в таблицу и освобождает память прямой модификацией MCB.

        Mov     al, 2Fh          ;В начале для каждого вектора проверка на перегрузку более поздними программами - если хотябы один вектор перехвачен, то выгружаться нельзя.
        Mov     dx, OFFSET New2Fh
        Call    Test_Vector
        Jnz     @@cant_unload

        Mov     al, 9h
        Mov     dx, OFFSET New9h
        Call    Test_Vector
        Jnz     @@cant_unload

        Mov     al, 13h
        Mov     dx, OFFSET New13h
        Call    Test_Vector
        Jnz     @@cant_unload

        Mov     al, 21h
        Mov     dx, OFFSET New21h
        Call    Test_Vector
        Jnz     @@cant_unload

        Mov     al, 25h
        Mov     dx, OFFSET New25h
        Call    Test_Vector
        Jnz     @@cant_unload

        Lds     dx, [cs:Old2Fh]    ;восстановление векторов
        Mov     al, 2Fh
        Call    RestoreVector

        Lds     dx, [cs:Old9h]
        Mov     al, 9h
        Call    RestoreVector

        Lds     dx, [cs:Old13h]
        Mov     al, 13h
        Call    RestoreVector

        Lds     dx, [cs:Old21h]
        Mov     al, 21h
        Call    RestoreVector

        Lds     dx, [cs:Old25h]
        Mov     al, 25h
        Call    RestoreVector

        Mov     bx, cs
        Dec     bx
        Mov     ds, bx
        Xor     bx, bx
        Mov     [word ptr (MCB bx).ownerPSP], 0  ;освобождение памяти

        Cmp     [word ptr cs:paramBlc+2], 0
        Jz      @@noParamsMem
        Mov     bx, [word ptr cs:paramBlc+2]
        Dec     bx
        Mov     ds, bx
        Xor     bx, bx
        Mov     [word ptr (MCB bx).ownerPSP], 0

@@noParamsMem:
        Mov     bx, [cs:2Ch] ;сегмент Env
        Dec     bx
        Mov     ds, bx
        Xor     bx, bx
        Mov     [word ptr (MCB bx).ownerPSP], 0  ;освобождение памяти блока Enviroment

        Push    cs
        Pop     ds
        Mov     si, OFFSET unlSound
        Call    Play

        Ret

@@cant_unload:
        Mov     di, ERR_RATE
        Mov     bx, ERR_DEL
        Call    Beep

        Ret

unlSound  DW  100,100, 200,100, 500,50, 900,20, 0,0

ENDP  Unload

PROC  RestoreVector  near

        Xor     ah, ah
        Shl     ax, 2
        Mov     bx, ax
        Xor     ax, ax
        Mov     es, ax
        Cli
        Mov     [es:bx], dx
        Mov     dx, ds
        Mov     [es:bx+2], dx
        Sti

        Ret

ENDP  RestoreVector


PROC Test_Vector near   ; al - vector, dx - offset my proc

        Xor     ah, ah
        Shl     ax, 2
        Mov     bx, ax
        Xor     ax, ax
        Mov     es, ax
        Cli
        Cmp     dx, [es:bx]
        Jnz     @@bad_vector
        Mov     dx, cs
        Cmp     dx, [es:bx+2]
@@bad_vector:

        Sti
        Ret

ENDP Test_Vector


ENDS  Code0


Segment  StkSeg  para public 'Code'
        myStk      DB  STK_SIZE  Dup('Stack')
ENDS StkSeg

Segment  Code2  word public 'Code'

ENDS  Code2


Segment  Code1  word public 'Code'

install:
        Push    cs
        Pop     ds

        Call    GetCPU
        Cmp     ax, REQVEST_PROCESSOR
        Jae     cpu_ok
        Mov     ah, 9h
        Mov     dx, OFFSET msgBadCPU
        Int     21h
        Int     20h

cpu_ok:

if PRODUCT GT 0
        Call    TestVirus       ;проверка контрольной суммы
endif

        Mov     ax, 1600h
        Int     2Fh
        Or      al, al
        Jz      @@canstart
        Push    cs
        Pop     ds
        Mov     ah, 9h
        Mov     dx, OFFSET msgWindowsVirtual
        Int     21h

        Mov     di, ERR_RATE
        Mov     bx, ERR_DEL
        Call    Beep

        Mov     ax, 4C02h
        Int     21h

@@canstart:
        Push    cs cs           ;проверка параметров командной строки
        Pop     es ds
        Mov     ax, cs
        Mov     [word ptr myPSPPtr+2], ax

        Lds     bx, [cs:myPSPPtr]
        Cmp     [(PSP bx).pSize], 0
        Jz      enter00
        Cmp     [(PSP bx).pSize], 3
        Jz      get_param
bad_param:
        Mov     ah, 9h
        Mov     dx, OFFSET msgBadParam
        Int     21h
        Int     20h

get_param:
        Cmp     [(PSP bx+2).parm], '?'
        Jz      helpOut
        Cmp     [(PSP bx+2).parm], 'H'
        Jz      helpOut
        Cmp     [(PSP bx+2).parm], 'h'
        Jz      helpOut
        Jmp     SHORT bad_param
helpOut:
        Mov     ah, 9h
        Mov     dx, OFFSET titl
        Int     21h
        Int     20h


enter00:
        Mov     ax, ASK_CODE       ;проверка повторной загрузки
        Int     2Fh
        Mov     di, OFFSET cs:pass
        Mov     cx, LEN00
        Cld
        REPZ Cmpsb
        Jz      already


        Cli                             ;переключение стека на свой
        Mov     ax, cs
        Mov     ss, ax
        Mov     sp, OFFSET MyCode:myStk
        Add     sp, STK_SIZE-16
        Sti

                            ;Free_Env
        Mov     ax, [cs:2Ch]
        Mov     [cs:paramsBlc.parsSeg], ax
        Mov     [cs:paramsBlc.cmdLinPtrS], cs
        Mov     [cs:paramsBlc.FCB1S], cs
        Mov     [cs:paramsBlc.FCB2S], cs

        ReDirectVector  2Fh              ;перехват векторов
        ReDirectVector  9h
        ReDirectVector  13h
        ReDirectVector  21h
        ReDirectVector  25h
        GetCritFlag     dosActive

        Call    TimerInit              ;инициализация процедуры игры музыки
        Push    cs
        Pop     ds
        Mov     ah, 9h
        Mov     dx, OFFSET titl
        Int     21h


        Mov     ah, 4Ah                     ;уменьшение программного блока памяти
        Mov     bx, OFFSET MyCode:end_start
        Shr     bx, 4
        Inc     bx
        Push    cs
        Pop     es
        Int     21h

        Call    ReadINI
        Jnc     @@okINI
        Push    cs
        Pop     ds
        Mov     ah, 9h
        Mov     dx, OFFSET errINIRead
        Int     21h
        Jmp     SHORT errExit_

@@okINI:

        ;Mov    ah, 48h                  ;резервирование памяти для антивируса
        ;Mov     bx, [cs:memSize]
        Mov     di, cs
        Mov     es, di
        Mov     di, OFFSET cs:dVectorTblSta
        Call    GetVTable
        ;Int    21h
ent1:
        Push    cs                  ;вывод заставки и разрешение работы обработчиков прерывания
        Pop     es
        Mov     ah, 9h
        Mov     dx, OFFSET msgInst
        Int     21h

        Mov     [cs:enblStart], 1
        Mov     [cs:enblGo], 1
        Mov     dx, OFFSET install
        Add     dx, 2
        Int     27h

errExit_:
        Mov     di, ERR_RATE
        Mov     bx, ERR_DEL
        Call    Beep

        UnRedirectVector  2Fh
        UnRedirectVector  9h
        UnRedirectVector  13h
        UnRedirectVector  21h
        UnRedirectVector  25h
        Push    cs
        Pop     ds

        Mov     ax, 4C03h
        Int     21h

already:
        Push    cs cs
        Pop     ds es

        Mov     ah, 9h
        Mov     dx, OFFSET msgInst2
        Int     21h

        Call    PrintConfig

        Mov     ax, 4C00h
        Int     21h


PROC  ReadINI near

        Mov     ax, [cs:2Ch]
        Mov     [cs:@@paramsBlc.parsSeg], ax
        Mov     [cs:@@paramsBlc.cmdLinPtrS], cs
        Mov     [cs:@@paramsBlc.FCB1S], cs
        Mov     [cs:@@paramsBlc.FCB2S], cs

        Mov     di, OFFSET  cs:@@strCom1
        Mov     si, OFFSET  cs:@@loadName
        Mov     cx, LEN_LNAME
        Call    Prepare_strWExec  ;IN: di - offs. out str; si - offs. name, cx - len name

        Mov     di, (OFFSET  cs:@@cmdLine) + 2
        Mov     si, OFFSET  cs:@@loadName2
        Mov     cx, LEN_LNAME2
        Call    Prepare_strWExec
        Mov     di, (OFFSET cs:@@cmdLine) + 1
        Call    StrLen
        Dec     cl
        Mov     [cs:@@cmdLine], cl

        Mov     [cs:@@OldSS], ss       ;сохранение стека
        Mov     [cs:@@OldSP], sp

        Mov     ax, 4B00h               ;DOS Exec
        Push    cs cs
        Pop     ds es
        Mov     dx, OFFSET MyCode:@@strCom1
        Mov     bx, OFFSET MyCode:@@paramsBlc
        Int     21h
        Lahf

        Cli                         ;восстановление стека
        Mov     ss, [cs:@@OldSS]
        Mov     sp, [cs:@@OldSP]
        Sti

        Sahf
        Jc      @@error
        Mov     ah, 4Dh
        Int     21h
        Or      al, al
        Jnz     @@error

        Ret
@@error:
        Stc
        Ret

@@strCom1    DB  131 Dup(0)
STA_LN = $
@@loadName   DB  'INILOAD.EXE', 0
LEN_LNAME = $ - STA_LN

@@cmdLine    DB  0, ' '
             DB  131 Dup(0)

STA_LN2 = $
@@loadName2   DB  'CHKFLOP.INI', 0Dh, 0
LEN_LNAME2 = $ - STA_LN2

@@OldSS   DW ?
@@OldSP   DW ?

@@fcb1    FCB <>
@@fcb2    FCB <>

@@paramsBlc    ParsTyp <, OFFSET MyCode:@@cmdLine,, OFFSET MyCode:@@fcb1,, OFFSET MyCode:@@fcb2,>

ENDP  ReadINI


myPSPPtr       PSP_PTR  0:?
msgBadParam    DB 10,13,"Ошибочные параметры, используйте /? для справки",10,13,'$'
msgBadCPU      DB 10,13,"Для работы зтой программы необходим 386-ой процессор или старше",10,13,7,'$'
msgWindowsVirtual DB  "Эта программа должна запускаться до загрузки Windows",10,13,'$'
errINIRead     DB  "Cann't CHKFLOP.INI read",10,13,'$'


titl  DB  10,13
      DB     "╔═══ (c)AlexCorp. ════════════════════════ Flop virus test V[2.0 alfa] ══╗",10,13
      DB     "║ All Rights Reserved                                    for free use    ║",10,13
      DB     "║                                                                        ║",10,13
      DB     "║               Автоматическое тестирование гибких дисков                ║",10,13
      DB     "║                    при замене носителя в дисководе                     ║",10,13
      DB     "║                                                                        ║",10,13
      DB     "║                     (c) Алексей Фёдоров 1995-96 г.                     ║",10,13
      DB     "║                                                                        ║",10,13
      DB     "╟── Горячие клавиши ─────────────────────────────────────────────────────╢",10,13
      DB     "║  Ctrl + Alt с любой стороны  +:                                        ║",10,13
      DB     "║                                                                        ║",10,13
      DB     "║    U                  - выгрузка программы из памяти.                  ║",10,13
      DB     "╟────────────────────────────────────────────────────────────────────────╢",10,13
      DB     "║ Тел. 146-7724, Факс ***-****, BBS ***-**** (**-** MSK), FIDO *:****/** ║",10,13
      DB     "╚════════════════════════════════════════════════════════════════════════╝",10,13,'$'

msgInst      DB     "   Программа инсталирована в памяти...",10,13,'$'
msgInst2     DB     "   Программа уже инсталирована в памяти...",10,13,'$'
msgNoMem     DB     "Мало памяти для использования антивируса",10,13,'$'

PROC  PrintConfig

        Push    ds ebx cx

        Mov     ax, GET_CONFIG
        Int     2Fh ;ebx
        Mov     ax, bx
        Shr     ebx, 16
        Mov     es, bx
        Mov     bx, ax

        Cmp     [es:(TParamBlock bx).opt.flCheckSerial], 0
        Jnz     @@yes
        Mov     [cs:@@YNo], '  oN'
@@yes:
        Push    cs
        Pop     ds
        Mov     ah, 9h
        Mov     dx, OFFSET @@msgSer
        Int     21h

        Call    _WordDec C, [es:(TParamBlock bx).opt.iSizeOfSerialQueue],\
                            OFFSET @@valBuf1, cs
        Mov     ah, 9h
        Mov     dx, OFFSET @@msgQueue
        Int     21h
        Mov     dx, OFFSET @@cr
        Int     21h

        Call    _WordDec C, [es:(TParamBlock bx).opt.iRetryNumberSec1ReadDOS],\
                            OFFSET @@valBuf2, cs
        Mov     ah, 9h
        Mov     dx, OFFSET @@msgRetDOS
        Int     21h
        Mov     dx, OFFSET @@cr
        Int     21h

        Call    _WordDec C, [es:(TParamBlock bx).opt.iRetryNumberSec1ReadWin],\
                            OFFSET @@valBuf3, cs
        Mov     ah, 9h
        Mov     dx, OFFSET @@msgRetWin
        Int     21h
        Mov     dx, OFFSET @@cr
        Int     21h


        Mov     dx, OFFSET @@DOSdsk
        Int     21h

        Mov     ax, [es:(TParamBlock bx).avL]
        Add     ax, OFFSET (TAVirusList).avEntyes
        Mov     [cs:@@avlOffs], ax

        Mov     si, [es:(TParamBlock bx).drLDOS]
        Mov     cx, [es:(TDrivesList si).iNumbEntries]
        Add     si, OFFSET (TDrivesList).deEntryes
        Call    @@ttyDrives

        Mov     ah, 9h
        Mov     dx, OFFSET @@Windsk
        Int     21h
        Mov     si, [es:(TParamBlock bx).drLWin]
        Mov     cx, [es:(TDrivesList si).iNumbEntries]
        Add     si, OFFSET (TDrivesList).deEntryes
        Call    @@ttyDrives

        Pop     cx ebx ds
        Ret

@@ttyDrives:
        Push    bx  ds ax
        Jcxz    @@end

        Push    es
        Pop     ds
        Mov     ah, 9h
        Mov     bl, 81h

@@loopDOS:
        Cmp     bl, [(TDriveEntry si).ucDriveDOS]
        Jnz     @@nextDrv
        Mov     di, [(TDriveEntry si).uiIndex]
        Push    ds cs
        Pop     ds
        Mov     dx, OFFSET @@spc
        Int     21h
        Pop     ds
        Mov     dx, [(TAVirEntry di).ptrStrCom]
        Int     21h
        Mov     dx, [(TAVirEntry di).ptrStrParam]
        Inc     dx
        Int     21h
        Push    ds cs
        Pop     ds
        Mov     dx, OFFSET @@cr
        Int     21h
        Pop     ds

        Add     si, SIZE TDriveEntry
        Loop    @@loopDOS

@@end:
        Pop     ax ds bx
        Retn

@@nextDrv:
        Mov     bl, [(TDriveEntry si).ucDriveDOS]
        Mov     [cs:@@Letter], bl
        Mov     dx, OFFSET @@msgDrv
        Push    ds cs
        Pop     ds
        Int     21h
        Pop     ds
        Jmp     SHORT @@loopDOS


@@avlOffs DW ?

@@msgSer  DB "Check serial = "
LABEL @@YNo DWORD
          DB "Yes ",10,13,'$'
@@DOSdsk  DB 10,13,"*** Check in DOS mode: ***",10,13,'$'
@@Windsk  DB 10,13,"*** Check in Windows: ***",10,13,'$'

@@msgDrv  DB "  Drive "
LABEL @@Letter BYTE
          DB "A:",10,13,'$'
@@spc     DB "    $"
@@cr      DB 10,13,'$'

@@msgQueue  DB "Size of serial queue = "
LABEL @@valBuf1 BYTE
            DB 25 Dup(0)

@@msgRetDOS DB "Retry number sec1 read in DOS = "
LABEL @@valBuf2 BYTE
            DB 25 Dup(0)

@@msgRetWin DB "Retry number sec1 read in Windows = "
LABEL @@valBuf3 BYTE
            DB 25 Dup(0)

ENDP  PrintConfig

ENDS  Code1

Segment  Code3  word public 'Code'
ENDS  Code3

Segment  Code4  word public 'Code'

File_length     DD              29372
File_CRC        DD              0F27A4E66h
end_start:

ENDS  Code4

        END

