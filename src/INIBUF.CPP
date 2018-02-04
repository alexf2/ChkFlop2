#include <stdio.h>
#include <cstring.h>
#include <typeinfo.h>
#include <classlib/file.h>

#include "iniload.hpp"
#include "autorc.hpp"
#include "inidta.hpp"


TBadParam::TBadParam( char *p1_, char *p2_, TINIBuf* pBufINIObj, char* pR ):
  p1( p1_ ), p2( p2_ ), pRange( pR )
 {
	char cBuf[300];

	if( !pBufINIObj ) return;

	setmem( cBuf, 300, 0 );
	strcpy( cBuf, "*** " );
	//pBufINIObj->GetString( cBuf + 5, 280 );
	strcat( cBuf, " ***\n" );

	strErrLine = cBuf;
 };


const char* TNoSection::Why() const
 {
	const static char *cStr = "Section %s in CHKFLOP.INI not found";
	static char cBuf[128];

	sprintf( cBuf, cStr, TINIBuf::GetSect(iSect) );
	return cBuf;
 }


const char* TDuplicateEntry::When() const
 {
	static char cBuf[300];

	sprintf( cBuf, "%s  %s=%s", pSect, pPar, pVal );
	return cBuf;
 }

const char *TINIBuf::cSections[ N_Sections ] =
 {
	"[AVirus]",
	"[DisksDOS]",
	"[DisksWin]",
	"[DOSMemory]",
	"[Options]"
 };

char* TINIBuf::operator[]( TSections iSection )
 {
	switch( iSection )
	 {
		case TAVirus:
		  return pAVirus;
		case TDisksDOS:
		  return pDisksDOS;
		case TDisksWin:
		  return pDisksWin;
		case TDOSMemory:
		  return pDOSMemory;
		case TOptions:
		  return pOptions;
		default:
		  break;
	 };

	return pBuf;
 }

void TINIBuf::Init()
 {
	pAVirus    = FindSection( TAVirus );
	pDisksDOS  = FindSection( TDisksDOS );
	pDisksWin  = FindSection( TDisksWin );
	pDOSMemory = FindSection( TDOSMemory );
	pOptions   = FindSection( TOptions );
 }
char *TINIBuf::FindSection( TSections iSection )
 {
	char *pIns = strstr( pBuf, cSections[iSection] );
	if( !pIns ) throw TNoSection( iSection );
	return pIns + strlen( cSections[iSection] );
 }

typedef char huge* PTHCh;
/*inline unsigned long Fl( char* p )
 {
	return ((unsigned long)FP_SEG(p)) << 4L +
			 (unsigned long)FP_OFF(p);
 }*/

void TINIBuf::GetString( char* pStr, int iMax )
 {
	*pStr = 0;          //pVPKey
	char* pEnd = pBuf + uiSize - 1;
	char *p1, *p2; p1 = p2 = pVPKey;

	if( (PTHCh)p1 < (PTHCh)pBuf || (PTHCh)p1 > (PTHCh)pEnd ||
		 (PTHCh)p2 < (PTHCh)p1 || (PTHCh)p2 > (PTHCh)(pEnd + 1) ) return;

	for( ; (PTHCh)p1 > (PTHCh)pBuf && *p1 != 10 && *p1 != 13; p1-- );
	for( ; (PTHCh)p2 < (PTHCh)pEnd && *p2 != 10 && *p2 != 13; p2++ );

	if( *p1 == 10 || *p1 == 13 ) p1++;
	if( *p2 == 10 || *p2 == 13 ) p2--;
	if( (PTHCh)p1 > (PTHCh)p2 ) p1 = p2;
	strncpy( pStr, p1, min(iMax, int(p2-p1)) );
 }

//extern TJmpFunc_ None_;
static TJmpFunc_ None_
 {
	return 0;
 }

static TJmpFunc_ gSta
 {
	return 1;
 }

static TJmpFunc_ gP
 {
	TINIBuf *pIB = (TINIBuf far*)pObj;

	char *pS = pStr - 1;

	*pIB->pParam = 0;
	int iSize = abs(pStrPrev - pS) + 1;
	if( iSize > pIB->iMaxPar ) throw TBadParam( pStrPrev, pS );
	strncpy( pIB->pParam, pStrPrev, iSize );
	pIB->pParam[ iSize ] = 0;

	return 0;
 }

static TJmpFunc_ gV
 {
	TINIBuf *pIB = (TINIBuf far*)pObj;

	char *pS = pStr - 1;

	*pIB->pVal = 0;
	int iSize = abs(pStrPrev - pS) + 1;
	if( iSize > pIB->iMaxVal ) throw TBadVal( pStrPrev, pS );
	strncpy( pIB->pVal, pStrPrev, iSize );
	pIB->pVal[ iSize ] = 0;

	return 0;
 }

static int Classify1INI( unsigned char bCh )
 {
	switch( bCh )
	 {
		case 0x0D: case 0x0A:
		  return 0;
		case '[': case ']':
		  return 1;
		case '=':
		  return 2;
		case 0:
		  return 4;
		case ' ':
		  return 5;
		default:
		  return 3;
	 };

	return -1;
 }

char* TINIBuf::GetPV( char *pSta, char *pParam_, int iMaxPar_, char* pVal_, int iMaxVal_ )
 {
	const int iNStr = 6, iNState = 7;
	static const TAutoDescr iTbl[ iNStr ][ iNState ] =
	 {
		{ { 0, None_}, {-1, None_}, {-1, None_}, {-1, None_}, {-1, None_}, { 6,    gV}, { 6, None_} },
		{ { 6, None_}, {-1, None_}, {-1, None_}, {-1, None_}, {-1, None_}, {-1, None_}, { 6, None_} },
		{ {-1, None_}, { 3,    gP}, { 3, None_}, {-1, None_}, {-1, None_}, { 5, None_}, { 6, None_} },
		{ { 1,  gSta}, { 1, None_}, {-1, None_}, { 5,  gSta}, { 5,  gSta}, { 5, None_}, { 6, None_} },
		{ { 6, None_}, {-1, None_}, {-1, None_}, {-1, None_}, {-1, None_}, { 6,    gV}, { 6, None_} },
		{ { 0, None_}, { 2,    gP}, { 2, None_}, { 4, None_}, { 4, None_}, { 5, None_}, { 6, None_} }
	 };

  pParam = pParam_; iMaxPar = iMaxPar_; pVal = pVal_; iMaxVal = iMaxVal_;

  pVPKey = AutoWork( pSta, strlen(pSta), (TAutoDescr far*)iTbl, Classify1INI,
							iNStr, iNState, this );
  return pVPKey;
 }

char* GetINIFile( char far* name, unsigned int& ruiSize )
 {
	TFile fINI( name, TFile::ReadOnly );

	if( fINI.GetHandle() == -1 ) throw TCanntRead();

	uint16 uiLen = fINI.Length();
	if( uiLen == -1 ) throw TCanntRead();

	TAutoMemoryRc<char*> ptINI( new char[uiLen + 1] );

	if( fINI.Read((char*)ptINI(), uiLen) != uiLen ) throw TCanntRead();
	char *pRet = ptINI();
	ptINI()[ uiLen ] = 0;
	ptINI.ClearRc();

	ruiSize = uiLen;
	return pRet;
 }

