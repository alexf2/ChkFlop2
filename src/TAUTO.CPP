#include <mem.h>
#include <string.h>

#include "autorc.hpp"
#include "tauto.hpp"


char* AutoWork( char far* pInp, int iLen, TJmpTable iTbl,
					 TClassifyFunc pClassify, int iNStr, int iNState, void far* pObj )
  //throw( TBadFormat, TInvalidChar, xalloc )
 {
	int iState = 0;
	char far* pStr, far* pStrPrev;
	pStr = pStrPrev = pInp;
	int iClass;

	bool bFlSta = true;
	for( int i = 1; iState != iNState - 1; pStr++, i++ )
	 {
		static char cTerm = 0;

		if( i > iLen )
		  iClass = pClassify( cTerm );
		else
		  iClass = pClassify( *pStr );

		if( iClass == -1 ) throw TInvalidChar( pStrPrev, pStr );

		TAutoDescr *pD = iTbl + (unsigned int)iClass*(unsigned int)iNState + (unsigned int)iState;

		int res = (pD->pF)( pStrPrev, pStr, pObj );

		if( (iState = pD->iNextState) == -1 ) throw TBadFormat( pStrPrev, pStr );
		//if( !bFlSta && *(pStr-1) == 0 ) pStrPrev = pStr;
		if( res ) pStrPrev = pStr;
		bFlSta = false;
	 }

	return pStr;
 }


