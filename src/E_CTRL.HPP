#if !defined(_EXEC_CONTROL_)
#define _EXEC_CONTROL_

#include <cstring.h>
#include "inidta.hpp"

class TExecControl
 {
public:
	TExecControl() { pWinList = 0; }

	void Ass( TParamBlock far* pBlc_ )
	 {
		pWinList = (TDrivesList far*)MK_FP( FP_SEG(pBlc_), pBlc_->pDrLWin );
	 }

	void Start( unsigned char cDrive_ );
	void Next( string& rStr, string& rPar );

private:
  TDrivesList far* pWinList;

  TDriveEntry far* pEntry;
  unsigned char cDrive;
  int iCount;
 };


#endif
