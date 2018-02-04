#if !defined(_TAUTOMAT_)
#define _TAUTOMAT_

#include <except.h>
#include <CLASSLIB/defs.h>


typedef int TJmpFunc_( char far* pStrPrev, char far* pStr, void far* pObj );
typedef TJmpFunc_ far *TJmpFunc;
typedef int (*TClassifyFunc)( unsigned char );

struct TAutoDescr
 {
	int iNextState;
	TJmpFunc pF;
 };

typedef TAutoDescr* TJmpTable;

class TAErr
 {
public:
  char far* p1, far* p2;

  TAErr( char far* p1_, char far* p2_ ):
	 p1( p1_ ), p2( p2_ )
	  {};
 };

class TInvalidChar: public TAErr
 {
public:
  TInvalidChar( char far* p1_, char far* p2_ ):
	 TAErr( p1_, p2_ )
	  {};
 };

class TBadFormat: public TAErr
 {
public:
  TBadFormat( char far* p1_, char far* p2_ ):
	 TAErr( p1_, p2_ )
	  {};
 };

char* AutoWork( char far* pInp, int iLen, TJmpTable, TClassifyFunc, int iNStr,
					 int iNState, void far* pObj );
					 //throw( TBadFormat, TInvalidChar, xalloc );

#endif
