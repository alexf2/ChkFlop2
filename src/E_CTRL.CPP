#include "e_ctrl.hpp"

void TExecControl::Start( unsigned char cDrive_ )
 {
	if( !pWinList ) return;

	cDrive = cDrive_;
	pEntry = pWinList->deEntryes;
	iCount = 0;
 }

void TExecControl::Next( string& rStr, string& rPar  )
 {
	if( !pWinList ) return;
	char cBuf[270];

	rStr = ""; //string
	while( iCount < pWinList->iNumbEntries )
	 {
		if( pEntry->ucDriveDOS == cDrive )
		 {
			TAVirusEntry far* pAV = (TAVirusEntry far*)MK_FP( FP_SEG(pEntry), pEntry->uiIndex );

			strcpy( cBuf, (char*)MK_FP(FP_SEG(pEntry),pAV->pStrCom) );
			rStr = cBuf;

			strcpy( cBuf, (char*)MK_FP(FP_SEG(pEntry),pAV->pStrParam+1) );
			cBuf[ strlen(cBuf) - 1 ] = 0;
			*strchr( cBuf, '#' ) = cDrive;
			rPar = cBuf;

			pEntry++; iCount++;
			return;
		 }
		pEntry++; iCount++;
	 };
 }

