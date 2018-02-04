
IDEAL

STRUC              DriveEntry
  ucDriveInt13   DB  0
  ucDriveDOS     DB  'A'        ;FFh - drive absent
  uiIndex        DW  1          ;index of antivirus str
ENDS               DriveEntry

TYPEDEF   TDriveEntry   DriveEntry

STRUC              DrivesList
  iNumbEntries   DW  0
  deEntryes      TDriveEntry  <>
ENDS               DrivesList

TYPEDEF   TDrivesList   DrivesList

STRUC              AVirEntry
  ucMem          DW  50  ;para = 16 bytes
  ptrStrCom      DW  0
  ptrStrParam    DW  0
ENDS               AVirEntry

TYPEDEF   TAVirEntry  AVirEntry

STRUC              AVirusList
  ucMemMax       DW  50
  iNumbAVirus    DW  0
  avEntyes       TAVirEntry  <>
ENDS               AVirusList

TYPEDEF   TAVirusList   AVirusList

STRUC              Options
  flCheckSerial           DB  1
  iSizeOfSerialQueue      DW  10
  iRetryNumberSec1ReadDOS DW  3
  iRetryNumberSec1ReadWin DW  5
ENDS               Options

TYPEDEF   TOptions  Options

STRUC               ParamBlock
  opt            TOptions <>
  avL            DW 0
  drLDOS         DW 0
  drLWin         DW 0
ENDS                ParamBlock

TYPEDEF   TParamBlock  ParamBlock
