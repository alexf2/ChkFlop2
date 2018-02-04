#if !defined(_COMPILE_)
#define _COMPILE_

#include "iniload.hpp"
#include "inidta.hpp"

void ReadOptions( TINIBuf& iniBuf, TOptions& opt );
void ReadAvirus( TINIBuf& iniBuf, TIDblListImpDel<TAVir>& liAvir );
void ReadDOSMem( TINIBuf& iniBuf, TIDblListImpDel<TMemDescr>& liDOSMem );
void ReadDisks( TINIBuf& iniBuf, TIDblListImpDel<TAVDescr>& liDisks,
					 TINIBuf::TSections iSect
				  );
void PrintAvir( TIDblListImpDel<TAVir>& liAvir );
void PrintDOSMem( TIDblListImpDel<TMemDescr>& liM );
void PrintDisks( TIDblListImpDel<TAVDescr>& liDsk );

#endif
