tasm /ml /m3 /q /dPRODUCT=1  chkflop2.asm + cpu2.asm + sound.asm + test_v.asm + scr.asm + worddec.asm
tlink /C /c /m /s /n /t  chkflop2.obj cpu2.obj sound.obj  test_v.obj  scr.obj worddec.obj
