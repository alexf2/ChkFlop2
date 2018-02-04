#include <stdio.h>

#include "autorc.hpp"
#include "compile.hpp"
#include "lim_opt.hpp"


static TIDblListImpDel<TAVir> _tmpl1_;
static TIDblListImpDel<TMemDescr> _tmpl2_;
static TIDblListImpDel<TAVDescr> _tmpl3_;


static int near pascal FindStr( char** pMatr, char* pStr, int iStrn )
 {
	for( int i = 0; i < iStrn; i++ )
	  if( !strcmpi(pMatr[i], pStr) )  return i;

	return -1;
 }

static void near pascal RemoveSpace( char* pStr )
 {
	char* pTmp = new char[ strlen(pStr) + 1 ];
	*pTmp = 0;
	char *pC1 = pStr, *pC2 = pTmp;
	while( *pC1 )
	 {
		if( *pC1 != ' ' ) *pC2++ = *pC1;
		pC1++;
	 }
	*pC2 = 0;

	strcpy( pStr, pTmp );
	delete []pTmp;
 }

static int near pascal ChkDigital( char far* cVal )
 {
	for( ; *cVal; cVal++ )
	  if( *cVal < '0' || *cVal > '9' ) return 0;

	return 1;
 }
enum { EX_BadParam = 0, EX_BadValue };
static unsigned int near pascal ValuePrm( char far* cVal,
														unsigned int uiMin,
														unsigned int uiMax,
														int iTypeExcept,
														TINIBuf& iniBuf
													 )
 {
	char cBuf[50];

	RemoveSpace( cVal );
	long int val = atol( cVal );
	if( !ChkDigital(cVal) || val < (long int)uiMin || val > (long int)uiMax )
	 {
		sprintf( cBuf, "Range: %u - %u", uiMin, uiMax );
		if( iTypeExcept == EX_BadParam )
		  throw TBadParam( cVal, cVal+strlen(cVal), &iniBuf, cBuf );
		else
		  throw TBadVal( cVal, cVal+strlen(cVal), &iniBuf, cBuf );
	 }

	return val;
 }


void ReadOptions( TINIBuf& iniBuf, TOptions& opt )
 {

	const char iNOpt = 4;
	static char *cOpt[ iNOpt ] =
	 {
		"CheckSerial",
		"SizeOfSerialQueue",
		"RetryNumberSec1ReadDOS",
		"RetryNumberSec1ReadWin"
	 };

	const int iMaxLenPar = 50, iMaxLenVal = 257;
	char cBPar[iMaxLenPar + 1], cBVal[iMaxLenVal + 1];

	char* pBuf = iniBuf[ TINIBuf::TOptions ];
	do {
	  *cBPar = 0; *cBVal = 0;
	  pBuf = iniBuf.GetPV( pBuf, cBPar, iMaxLenPar,
		 cBVal, iMaxLenVal
								);
	  if( *cBPar )
		{
		  int iInd = FindStr( cOpt, cBPar, iNOpt );
		  if( iInd < 0 ) throw TBadParam( cBPar, cBPar+strlen(cBPar), &iniBuf );

		  unsigned int uiVal;
		  switch( iInd )
			{
			  case 0:
				 RemoveSpace( cBVal );
				 if( !strcmpi(cBVal, "Yes") ) opt.flCheckSerial = 1;
				 else
					if( !strcmpi(cBVal, "No") ) opt.flCheckSerial = 0;
				 else
					throw TBadVal( cBVal, cBPar+strlen(cBVal), &iniBuf );
				 break;

			  case 1:
				 uiVal = ValuePrm( cBVal, LIM_Queue_MIN,
					LIM_Queue_MAX, EX_BadValue, iniBuf );
				 opt.iSizeOfSerialQueue = uiVal;
				 break;

			  case 2:
				 uiVal = ValuePrm( cBVal, LIM_Retry_MIN,
					LIM_Retry_MAX, EX_BadValue, iniBuf );
				 opt.iRetryNumberSec1ReadDOS = uiVal;
				 break;

			  case 3:
				 uiVal = ValuePrm( cBVal, LIM_Retry_MIN,
					 LIM_Retry_MAX, EX_BadValue, iniBuf );
				 opt.iRetryNumberSec1ReadWin = uiVal;
				 break;
			};
		}
	  } while( *cBPar );
 }


void ReadAvirus( TINIBuf& iniBuf, TIDblListImpDel<TAVir>& liAvir )
 {
	const int iMaxLenPar = 5, iMaxLenVal = 257;
	char cBPar[iMaxLenPar + 1], cBVal[iMaxLenVal + 1];

	char* pBuf = iniBuf[ TINIBuf::TAVirus ];
	do {
	  *cBPar = 0; *cBVal = 0;
	  pBuf = iniBuf.GetPV( pBuf, cBPar, iMaxLenPar,
		 cBVal, iMaxLenVal
								);
	  if( *cBPar )
		{
		  unsigned int val = ValuePrm( cBPar, LIM_Index_MIN,
			 LIM_Index_MAX, EX_BadParam, iniBuf );

		  TAutoMemoryRc<TAVir*> rcAV( new TAVir( cBVal, val ) );

		  if( liAvir.Find2(rcAV()) ) throw TDuplicateEntry( cBPar,
			 cBVal, TINIBuf::GetSect(TINIBuf::TAVirus) );

		  TAVir* pTmp = rcAV(); rcAV.ClearRc();
		  liAvir.Add( pTmp );
		}
	  } while( *cBPar );
 }

void ReadDOSMem( TINIBuf& iniBuf, TIDblListImpDel<TMemDescr>& liDOSMem )
 {
	const int iMaxLenPar = 5, iMaxLenVal = 10;
	char cBPar[iMaxLenPar + 1], cBVal[iMaxLenVal + 1];

	char* pBuf = iniBuf[ TINIBuf::TDOSMemory ];
	do {
	  *cBPar = 0; *cBVal = 0;
	  pBuf = iniBuf.GetPV( pBuf, cBPar, iMaxLenPar,
		 cBVal, iMaxLenVal
								);
	  if( *cBPar )
		{
		  unsigned int val = ValuePrm( cBPar, LIM_Index_MIN,
			LIM_Index_MAX, EX_BadParam, iniBuf );

		  unsigned int val2 = ValuePrm( cBVal, LIM_DOSMem_MIN,
			 LIM_DOSMem_MAX, EX_BadValue, iniBuf );

		  TMemDescr tmpM(val, val2);
		  if( liDOSMem.Find2(&tmpM) ) throw TDuplicateEntry( cBPar,
			 cBVal, TINIBuf::GetSect(TINIBuf::TDOSMemory) );
		  liDOSMem.Add( new TMemDescr(val, val2) );
		}
	  } while( *cBPar );
 }

void ReadDisks( TINIBuf& iniBuf, TIDblListImpDel<TAVDescr>& liDisks,
					 TINIBuf::TSections iSect
				  )
 {
	const int iMaxLenPar = 2, iMaxLenVal = 257;
	char cBPar[iMaxLenPar + 1], cBVal[iMaxLenVal + 1];

	char* pBuf = iniBuf[ iSect ];
	do {
	  *cBPar = 0; *cBVal = 0;
	  pBuf = iniBuf.GetPV( pBuf, cBPar, iMaxLenPar,
		 cBVal, iMaxLenVal
								);
	  if( *cBPar )
		{
		  static char cBadP[270];

		  RemoveSpace( cBPar );
		  strupr( cBPar );
		  if( *cBPar < 'A' || *cBPar > 'Z' )
			 throw TBadParam( cBPar, cBPar+strlen(cBPar), &iniBuf );

		  TAutoMemoryRc<TAVDescr*> rcDrive( new TAVDescr(int(*cBPar)) );
		  if( liDisks.Find2(rcDrive()) ) throw TDuplicateEntry( cBPar,
			 cBVal, TINIBuf::GetSect(iSect) );

		  RemoveSpace( cBVal );
		  if( !*cBVal )
			{
			  sprintf( cBadP, "%s=%s", cBPar, cBVal );
			  throw TBadVal( cBadP, cBadP+strlen(cBadP),  &iniBuf );
			}
		  char *pS1, *pS2;
		  pS1 = pS2 = cBVal;
		  for( ; 1; pS2++ )
			{
			  if( *pS2 == ',' || !*pS2 )
				{
				  char cTmp[256];
				  strncpy( cTmp, pS1, abs(pS1-pS2) );
				  cTmp[ abs(pS1-pS2) ] = 0;
				  int iVal = atoi( cTmp );
				  if( !*cTmp || iVal < 1 )
					 throw TBadVal( cBVal, cBPar+strlen(cBVal), &iniBuf );

				  pS1 = pS2 + 1;
				  (*rcDrive()) += iVal;
				}

			  if( !*pS2 ) break;
			}

		  TAVDescr* pTmp = rcDrive(); rcDrive.ClearRc();
		  liDisks.Add( pTmp );
		}
	  } while( *cBPar );
 }


void PrintAvir( TIDblListImpDel<TAVir>& liAvir )
 {
	char cB[257];

	TISDoubleListIteratorImp<TAVir> liIter( liAvir );

	TAVir *pAV = liIter.Current();
	for( ; int(liIter); pAV =  ++liIter )
	 {
		pAV->GetFullStr( cB, 256 );
		printf( "\t%d=%s\n", int(*pAV), cB );
	 }
 }

void PrintDOSMem( TIDblListImpDel<TMemDescr>& liM )
 {
	TISDoubleListIteratorImp<TMemDescr> liIter( liM );

	TMemDescr *pAV = liIter.Current();
	for( ; int(liIter); pAV =  ++liIter )
	  printf( "\t%d=%u\n", int(*pAV), pAV->uiMem );
 }

void PrintDisks( TIDblListImpDel<TAVDescr>& liDsk )
 {
	TISDoubleListIteratorImp<TAVDescr> liIter( liDsk );

	TAVDescr *pAV = liIter.Current();
	for( ; int(liIter); pAV =  ++liIter )
	 {
		printf( "\t%c=", (char)int(*pAV) );

		TSDoubleListIteratorImp<int> liIter2( pAV->GetChain() );
		int iVal = liIter2.Current();
		for( ; int(liIter2); iVal =  ++liIter2 )
		  printf( "%d,", iVal );

		printf( "\n" );
	 }
 }


