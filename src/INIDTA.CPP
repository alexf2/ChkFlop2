#include <typeinfo.h>
#include <stdio.h>
#include <CLASSLIB/dlistimp.h>
#include <CLASSLIB/sets.h>
#include <CLASSLIB/file.h>

#include "inidta.hpp"
#include "tauto.hpp"
#include "compile.hpp"

#if defined(CHK_WINDOWS)
  #include <windows.h>
#endif

//class TListImp<int>;

void TAVDescr::operator+=( int iAvir )
 {
	if( !liAvir.Find2(iAvir) ) liAvir.Add( iAvir );
 }

void TAVDescr::operator-=( int iAvir )
 {
	if( liAvir.Find2(iAvir) ) liAvir.Detach( iAvir );
 }


static int Classify( unsigned char bCh )
 {
	switch( bCh )
	 {
		case ':':  return 1;
		case '#':  return 2;
		case '\\': return 3;
		case '.':  return 4;
		case '/':  return 6;
		case ' ':  return 7;
		case 0:   return 8;
		case '?': case '*':  return 5;
		default:
		  if( bCh >= 0x21 && bCh <= 0x29 ||
				bCh >= 0x2B && bCh <= 0x3E ||
				bCh >= 0x40 && bCh <= 0x5B ||
				bCh >= 0x5D && bCh <= 0xFE
			 ) return 0;
	 };

	return -1;
 }
//TInvalidChar, TBadFormat

static TJmpFunc_ Path_
 {
  TAVir *pAV = (TAVir far*)pObj ;
  char *pS = pStr - 1;
	pAV->strPath = string( pStrPrev, 0, abs(pS - pStrPrev) + 1 );
	//*(pStr-1) = 0;

	return 1;
 }

static TJmpFunc_ Lbl_
 {
	//*(pStr-1) = 0;
	return 1;
 }

static TJmpFunc_ None_
 {
	return 0;
 }

static TJmpFunc_ Mask_
 {
	TAVir *pAV = (TAVir far*)pObj;
	char *pS = pStr - 1;
	pAV->strMask = string( pStrPrev, 0, abs(pS - pStrPrev) + 1 );
	//*(pStr-1) = 0;

	return 1;
 }

static TJmpFunc_ Prefix_
 {
	TAVir *pAV = (TAVir far*)pObj;
	char *pS = pStr - 1;
	pAV->strPrefix = string( pStrPrev, 0, abs(pS - pStrPrev) + 1 );
	//*(pStr-1) = 0;

	return 1;
 }

static TJmpFunc_ PostFix_
 {
	TAVir *pAV = (TAVir far*)pObj;
	char *pS = pStr - 1;
	pAV->strPostfix = string( pStrPrev, 0, abs(pS - pStrPrev) + 1 );
	//*(pStr-1) = 0;

	return 1;
 }

static TJmpFunc_ gSta
 {
	return 1;
 }

void TAVir::Split( const char far* pStr__ )
 {
	//D:\AIDS\aidstest.exe   /k #:\*.*  /f/s
	const int iNStr = 9, iNState = 7;
	static const TAutoDescr iTbl[ iNStr ][ iNState ] =
	 {
		{ {0, None_},  {1, None_},   {-1, None_}, {-1, None_}, {4, None_}, {5, None_} },
		{ {0, None_},  {1, None_},   {3, None_},  {-1, None_}, {-1, None_},{5, None_} },
		{ {0, None_},  {2, Prefix_}, {-1, None_}, {-1, None_}, {-1, None_},{5, None_} },
		{ {0, None_},  {1, None_},   {-1, None_}, {4, Lbl_},   {-1, None_},{5, None_} },
		{ {0, None_},  {1, None_},   {-1, None_}, {-1, None_}, {4, None_}, {5, None_} },
		{ {-1, None_}, {1, None_},   {-1, None_}, {4, Lbl_},   {4, None_}, {5, None_} },
		{ {-1, None_}, {1, None_},   {-1, None_}, {5, Lbl_},   {5, Mask_}, {5, None_} },
		{ {1, Path_},  {1, None_},   {-1, None_}, {5, Lbl_},   {5, Mask_}, {5, None_} },
		{ {-1, None_}, {-1, None_},  {-1, None_},  {6, None_},  {6, Mask_}, {6, PostFix_} }
	 };

	//char cTmp[ 257 ];
	//memset( cTmp, 256, 0 );
	//strncpy( cTmp+1, pStr__, 255 );

	strPath     = string( "aidstest.exe" );
	strPrefix   = string( "" );
	strPostfix  = string( "/s" );
	strMask     = string( "*.*" );

	AutoWork( const_cast<char far*>(pStr__), strlen(pStr__), (TAutoDescr far*)iTbl, Classify, iNStr, iNState, this );
 }

void TAVir::GetFullStr( char far* pDst, int iMax ) const
 {
	string strTmp = strPath + strPrefix + "#:" + strMask + strPostfix;
	strncpy( pDst, strTmp.c_str(), iMax );
	pDst[ iMax ] = 0;
 }

void TAVir::GetExecParams( char far* pStr, char far* pParams ) const
 {
	strcpy( pStr, strPath.c_str() );
	int iLen = strlen( pStr ); pStr[iLen+1] = '$';

	strcpy( pParams + 1, strPrefix.c_str() );
	strcat( pParams + 1, "#:" );
	strcat( pParams + 1, strMask.c_str() );
	strcat( pParams + 1, strPostfix.c_str() );
	iLen = strlen( pParams + 1 ); *pParams = (unsigned char)iLen;
	(pParams + 1)[iLen] = 0x0D; (pParams + 1)[iLen+1] = 0;
	(pParams + 1)[iLen+2] = '$';
 }

void TParamBlock_Load::Init( TINIBuf& iniBuf )
 {
	ReadOptions( iniBuf, opt );
	ReadDOSMem( iniBuf, liDOSMem );
	ReadDisks( iniBuf, liDisksDOS, TINIBuf::TDisksDOS );
	ReadDisks( iniBuf, liDisksWin, TINIBuf::TDisksWin );
	ReadAvirus( iniBuf, liAvir );
 }


static int CondInd( const TDescriptor& rD, void* ind )
 {
	return int(rD) == int(ind);
 }
typedef int (*TCondInd_DOSMem)( const TMemDescr&, void* );
typedef int (*TCondInd_AVir)( const TAVir&, void* );

static bool near pascal CheckDsks( TIDblListImpDel<TAVir>& rLAV,
											  TIDblListImpDel<TAVDescr>& rLDrv,
											  int& iRef
											)
 {
	TISDoubleListIteratorImp<TAVDescr> iter( rLDrv );
	TAVDescr *pDescr = iter.Current();
	for( ; int(iter); pDescr = ++iter )
	 {
		unsigned char cCode = int(*pDescr) - 'A' + 1;
		int iTst;
		char cMsg[80];

#if defined(CHK_WINDOWS)
	  iTst = GetDriveType( cCode - 1 ) == DRIVE_REMOVABLE;
#else
	asm {
		  Push   ax
		  Push   bx

		  Mov	 	ax, 4408h
		  Mov	 	bl, cCode
		  Int	   21h
		  Jc     ERR_NODISK
		  Mov	   ah, 0
		  Mov	   iTst, ax

		  Pop    bx
		  Pop    ax
		  Jmp    OK
	 };
ERR_NODISK:
  sprintf( cMsg, "CHKFLOP:  Drive %c: is absent\n",
						char(int(*pDescr)) );
  throw( xmsg(cMsg) );
OK:
	iTst = !iTst;
#endif

		if( !iTst )
		 {
			sprintf( cMsg, "CHKFLOP:  Drive %c: isn't a floppy drive\n",
						char(int(*pDescr)) );
			throw( xmsg(cMsg) );
		 }

		TSDoubleListIteratorImp<int> iter2( pDescr->GetChain() );
		int iChain = iter2.Current();
		for( ; int(iter2); iChain = ++iter2 )
		  if( !rLAV.FirstThat((TCondInd_AVir)CondInd, (void*)iChain ) )
			{
			  iRef = iChain;
			  return false;
			}
	 }

	return true;
 }

static void ThrowInvalidRef( const char *pSect, int iSect )
 {
	char cBuf[128];
	sprintf( cBuf, "CHECKFLOP: INI file %s section contain invalid antivirus "
						"reference '%d'\n", pSect, iSect
			 );
	throw xmsg( cBuf );
 }

void TParamBlock_Load::Check() throw( xmsg )
 {
	TISDoubleListIteratorImp<TAVir> iter( liAvir );
	TAVir *pAvir = iter.Current();
	char cBuf[128];

	for( ; int(iter); pAvir = ++iter )
	  if( !liDOSMem.FirstThat((TCondInd_DOSMem)CondInd, (void*)int(*pAvir)) )
		{
		  sprintf( cBuf,
		  "CHECKFLOP: INI file [DOSMemory] section must contain synhronize entry"
		  " for '%d=' [AVirus]\n",
		  int(*pAvir) );
		  throw xmsg( cBuf );
		}

	TISDoubleListIteratorImp<TMemDescr> iter2( liDOSMem );
	TMemDescr *pM = iter2.Current();

	for( ; int(iter2); pM = ++iter2 )
	  if( !liAvir.FirstThat((TCondInd_AVir)CondInd, (void*)int(*pM)) )
		{
		  sprintf( cBuf,
		  "CHECKFLOP: INI file [DOSMemory] section must contain only entryes"
		  " synhronized with [AVirus] (%d= - is not)\n",
		  int(*pM) );
		  throw xmsg( cBuf );
		}

	int iRef;
	if( !CheckDsks( liAvir, liDisksDOS, iRef ) )
	  ThrowInvalidRef( "[DisksDOS]", iRef );
	if( !CheckDsks( liAvir, liDisksWin, iRef ) )
	  ThrowInvalidRef( "[DisksWin]", iRef );
 }

unsigned long TParamBlock_Load::SzAvirList()
 {
	unsigned long ulSize = sizeof(TAVirusList);

	if( liAvir.GetItemsInContainer() > 1 )
	  ulSize += ((unsigned long)liAvir.GetItemsInContainer()-1L) *
					(unsigned long)sizeof(TAVirusEntry);

	TISDoubleListIteratorImp<TAVir> iter( liAvir );
	TAVir *pAV = iter.Current();
	for( ; int(iter); pAV = ++iter )
	 {
		char cStr[270], cParm[270];
		pAV->GetExecParams( cStr, cParm );
		ulSize += (unsigned long)strlen( cStr ) + 2L +
					 (unsigned long)strlen( cParm + 1 ) + 3L;
	 }

	return ulSize;
 }

int TParamBlock_Load::CalcDriveDescrs( TIDblListImpDel<TAVDescr>& liDriv )
 {
	int iNumb = 0;

	TISDoubleListIteratorImp<TAVDescr> iter( liDriv );
	TAVDescr *pAV = iter.Current();
	for( ; int(iter); pAV = ++iter )
	  iNumb += pAV->GetChain().GetItemsInContainer();

	return iNumb;
 }

unsigned long TParamBlock_Load::SzDriveList( TIDblListImpDel<TAVDescr>& liDriv )
 {
	unsigned long ulSize = sizeof(TDrivesList);
	int iChains = CalcDriveDescrs( liDriv );
	if( iChains > 1 )
	  ulSize += ((unsigned long)iChains-1L) * (unsigned long)sizeof(TDriveEntry);

	return  ulSize;
 }

unsigned int TParamBlock_Load::GetLowLevelSize()
 {
	return (((unsigned long)sizeof(TParamBlock) + SzAvirList() +
			 SzDriveList(liDisksDOS) + SzDriveList(liDisksWin)) / 16L + 1L);
 }

static void Iter_MaxMem( TMemDescr& rAV, void* pMax )
 {
	*(unsigned int*)pMax = max( *(unsigned int*)pMax, rAV.uiMem );
 }

void TParamBlock_Load::InitTSROwnMem( TParamBlock  far* pParBlc )
 {
/*  TParamBlock
 {
	TOptions     opt;
	TAVirusList near*  pAvL;
	TDrivesList near* pDrLDOS;
	TDrivesList near* pDrLWin;
 };*/
	int iChains = CalcDriveDescrs( liDisksDOS ),
		 iChains2 = CalcDriveDescrs( liDisksWin );
	unsigned int uiSeg = FP_SEG( pParBlc );
	unsigned int
	  uiSizeAVList = (liAvir.GetItemsInContainer() > 1) ?
		 (((unsigned long)liAvir.GetItemsInContainer()-1L) *
		 (unsigned long)sizeof(TAVirusEntry)) : 0,

	  uiSizeDOSDrList = (iChains > 1) ?
		 (((unsigned long)iChains-1L) *
		 (unsigned long)sizeof(TDriveEntry)) : 0,

	  uiSizeWinDrList = (iChains2 > 1) ?
		 (((unsigned long)iChains2-1L) *
		 (unsigned long)sizeof(TDriveEntry)) : 0;


	pParBlc->opt  = opt;
	pParBlc->pAvL = (TAVirusList near*)(FP_OFF(pParBlc) + sizeof(TParamBlock));
	pParBlc->pDrLDOS = (TDrivesList near*)(((unsigned int)pParBlc->pAvL) +
	  sizeof(TAVirusList) + uiSizeAVList);
	pParBlc->pDrLWin = (TDrivesList near*)(((unsigned int)pParBlc->pDrLDOS) +
	  sizeof(TDrivesList) + uiSizeDOSDrList);


	TAVirusList far* pAvl = (TAVirusList far*)MK_FP( uiSeg,
									  (unsigned int)(pParBlc->pAvL) );
	TDrivesList far* pDrLDOS = (TDrivesList far*)MK_FP( uiSeg,
									  (unsigned int)(pParBlc->pDrLDOS) );
	TDrivesList far* pDrLWin = (TDrivesList far*)MK_FP( uiSeg,
									  (unsigned int)(pParBlc->pDrLWin) );

	pAvl->ucMemMax = 0;
	liDOSMem.ForEach( Iter_MaxMem, &(pAvl->ucMemMax) );
	pAvl->iNumbAVirus = liAvir.GetItemsInContainer();

	pAvl->avEntryes[0].pStrCom = (char near*)(((unsigned int)pParBlc->pDrLWin) +
	  sizeof(TDrivesList) + uiSizeWinDrList);
	char far* pAVStr = (char far*)MK_FP( uiSeg,
							 (unsigned int)(pAvl->avEntryes[0].pStrCom) );


	TAVirusEntry far* pAVEntry = pAvl->avEntryes;
	TISDoubleListIteratorImp<TAVir> iter( liAvir );
	TAVir *pAV = iter.Current();
	for( ; int(iter); pAV = ++iter, pAVEntry++ )
	 {
		char cStr[270], cParm[270];
		pAV->GetExecParams( cStr, cParm );

		TMemDescr mTmp( int(*pAV), 0 );
		//TMemDescr* mTMp = liDOSMem.Find2( &mTmp );
		pAVEntry->uiMem = liDOSMem.Find2( &mTmp )->uiMem;

		int iS;
		pAVEntry->pStrCom = (char  near*)FP_OFF( pAVStr );
		memcpy( pAVStr, cStr, (iS=strlen(cStr) + 2) );
		pAVStr += iS;

		pAVEntry->pStrParam = (char  near*)FP_OFF( pAVStr );
		memcpy( pAVStr, cParm, (iS=strlen(cParm+1) + 3) );
		pAVStr += iS;
	 }

	pDrLDOS->iNumbEntries = iChains;
	pDrLWin->iNumbEntries = iChains2;
	FillDiskEntrys( pDrLDOS->deEntryes, liDisksDOS, pAvl->avEntryes );
	FillDiskEntrys( pDrLWin->deEntryes, liDisksWin, pAvl->avEntryes );
 }

 /*struct TAVirusEntry
 {
	uint16 uiMem;
	char  near* pStrCom;    //str, 0, $
	char  near* pStrParam; // SIZE, str, 0Dh, 0, $
								  //  1b ,    , 1b
 };*/

void TParamBlock_Load::FillDiskEntrys( TDriveEntry far* pDrEntry,
  TIDblListImpDel<TAVDescr>& rLst, TAVirusEntry far* pAVEntry )
 {
	TISDoubleListIteratorImp<TAVDescr> iter( rLst );
	TAVDescr *pDr = iter.Current();
	for( ; int(iter); pDr = ++iter )
	 {
		TSDoubleListIteratorImp<int> iter2( pDr->GetChain() );
		int iChain = iter2.Current();
		for( ; int(iter2); iChain = ++iter2, pDrEntry++ )
		  {
			 pDrEntry->ucDriveInt13 = (unsigned char)int(*pDr) - 'A';
			 pDrEntry->ucDriveDOS   = (unsigned char)int(*pDr);
			 pDrEntry->uiIndex      = FP_OFF(pAVEntry + iChain - 1);
		  }
	 }
 }

void TParamBlock_Load::RenumRefs()
 {
	int iNum = 1;
	TISDoubleListIteratorImp<TAVir> iter( liAvir );
	TAVir *pAV = iter.Current();
	for( ; int(iter); pAV = ++iter, iNum++ )
	 {
		if( iNum == int(*pAV) ) continue;

		TMemDescr mTmp( int(*pAV), 0 );
		TMemDescr *pMemDr = liDOSMem.Find2( &mTmp );
		pMemDr->ChgID( iNum );

		RenumRefsDrv( liDisksDOS, int(*pAV), iNum );
		RenumRefsDrv( liDisksWin, int(*pAV), iNum );

		pAV->ChgID( iNum );
	 }
 }

void TParamBlock_Load::RenumRefsDrv( TIDblListImpDel<TAVDescr>& rLst, int iOld, int iNew )
 {
	TISDoubleListIteratorImp<TAVDescr> iter2( rLst );
	TAVDescr *pDr = iter2.Current();
	for( ; int(iter2); pDr = ++iter2 )
	 {
		TSDoubleListIteratorImp<int> iter( pDr->GetChain() );
		int iChain = iter.Current();
		for( ; int(iter); iChain = ++iter )
		  if( iOld == iChain )
			{
			  pDr->GetChain().Detach( iChain );
			  pDr->GetChain().Add( iNew );
			  break;
			}
	 }
 }

static int IndexCond( const TAVir& rAv, void* iInd )
 {
	return int(rAv) == int(iInd);
 }

int TParamBlock_Load::GetFreeIndex()
 {
	int iIndex = 1;
	for( ; liAvir.FirstThat(IndexCond, (void*)iIndex); iIndex++ );
	return iIndex;
 }


static char far* pNameINIActive = 0;
static void near pascal CWri( TFile& rFile, char* pStr )
 {
	int iLen;
	if( rFile.Write(pStr, (iLen = strlen(pStr))) != iLen )
	 {
		char cBuf[300];
		sprintf( cBuf, "Error write: '%s' - %s", pNameINIActive,
					_sys_errlist[_doserrno] );
		throw xmsg( cBuf );
	 }
 }

static void near pascal WriSection( TIDblListImpDel<TAVDescr>& liDsks,
												char* cSection,
												TFile& rFile
											 )
 {
	char cBuf[300];

	CWri( rFile, cSection );
	TISDoubleListIteratorImp<TAVDescr> iter( liDsks );
	TAVDescr *pDsks = iter.Current();
	for( ; int(iter); pDsks = ++iter )
	 {
		sprintf( cBuf, "  %c = ", (char)int(*pDsks) );
		CWri( rFile, cBuf );

		bool bFl = true;
		TSDoubleListIteratorImp<int> iter2( pDsks->GetChain() );
		int iAv = iter2.Current();
		for( ; int(iter2); iAv = ++iter2 )
		 {
			if( !bFl )
			 {
				sprintf( cBuf, "," );
				CWri( rFile, cBuf );
			 } else bFl = false;
			sprintf( cBuf, " %u", iAv );
			CWri( rFile, cBuf );
		 }
		CWri( rFile, "\r\n" );
	 }
 }


void TParamBlock_Load::Save( char *fName )
 {
	pNameINIActive = fName;
	TFile fINI( fName, TFile::WriteOnly | TFile::Create );

	char cBuf[300];
	if( fINI.GetHandle() == -1 )
	 {
		sprintf( cBuf, "Error write: '%s' - %s", fName,  _sys_errlist[_doserrno] );
		throw xmsg( cBuf );
	 }

	CWri( fINI, "[AVirus]\r\n" );
	TISDoubleListIteratorImp<TAVir> iter( liAvir );
	TAVir *pAV = iter.Current();
	char cB[270];
	for( ; int(iter); pAV = ++iter )
	 {
		pAV->GetFullStr( cB, 269 );
		sprintf( cBuf, "  %u = %s\r\n", int(*pAV), cB );
		CWri( fINI, cBuf );
	 }

	CWri( fINI, "\r\n[DOSMemory]\r\n" );
	TISDoubleListIteratorImp<TMemDescr> iter2( liDOSMem );
	TMemDescr *pM = iter2.Current();
	for( ; int(iter2); pM = ++iter2 )
	 {
		sprintf( cBuf, "  %u = %u\r\n", int(*pM), pM->uiMem );
		CWri( fINI, cBuf );
	 }

	WriSection( liDisksDOS, "\r\n[DisksDOS]\r\n", fINI );
	WriSection( liDisksWin, "\r\n[DisksWin]\r\n", fINI );

	CWri( fINI, "\r\n[Options]\r\n" );
	sprintf( cBuf, "  CheckSerial = %s\r\n",
				opt.flCheckSerial ? "Yes":"No");
	CWri( fINI, cBuf );

	sprintf( cBuf, "  SizeOfSerialQueue = %u\r\n", opt.iSizeOfSerialQueue );
	CWri( fINI, cBuf );

	sprintf( cBuf, "  RetryNumberSec1ReadDOS = %u\r\n", opt.iRetryNumberSec1ReadDOS );
	CWri( fINI, cBuf );

	sprintf( cBuf, "  RetryNumberSec1ReadWin = %u\r\n", opt.iRetryNumberSec1ReadWin );
	CWri( fINI, cBuf );
 }


