#if !defined(_INI_LOAD_)
#define _INI_LOAD_

#include <cstring.h>
//#include "inidta.hpp"
#include "tauto.hpp"

class TNoSection;
class TBadParam;
class TDuplicateEntry;
class TBadVal;
class TCanntRead {};

char* GetINIFile( char far* name, unsigned int& ruiSize );

#define N_Sections 5
class TINIBuf
 {
friend TJmpFunc_ gP;
friend TJmpFunc_ gV;

public:

  enum TSections { TAVirus = 0, TDisksDOS, TDisksWin, TDOSMemory, TOptions };

  TINIBuf( char far* pBuf_, unsigned int uiSize_ ):
	 pBuf( pBuf_ ),
	 uiSize( uiSize_ )
	 {
      pVPKey = 0;
		pAVirus = pDisksDOS = pDisksWin = pDOSMemory = pOptions = 0;
		Init();
	 }

  operator char*() { return pBuf; }
  char* operator []( TSections iSection );
  static const char* GetSect( TSections iSection ) { return cSections[iSection]; }

  char* GetPV( char *pSta, char *pParam, int iMaxPar, char* pVal, int iMaxVal );
  void GetString( char* pStr, int iMax );

protected:
  void Init();
  char *FindSection( TSections iSection );

  char *pParam, *pVal;
  int iMaxPar, iMaxVal;

private:
  char far* pBuf;
  char far* pVPKey;
  unsigned int uiSize;

  char far* pAVirus;
  char far* pDisksDOS;
  char far* pDisksWin;
  char far* pDOSMemory;
  char far* pOptions;

  const static char *cSections[ N_Sections ];
 };


class TNoSection
 {
public:
  TNoSection( TINIBuf::TSections iSect_ ):
	 iSect( iSect_ )
	  {}

  const char* Why() const;

private:
  TINIBuf::TSections iSect;
 };


class TBadParam
 {
public:
  char *p1, *p2;
  char *pRange;
  string strErrLine;

  TBadParam( char *p1_, char *p2_, TINIBuf* pBufINIObj = 0, char* pR = 0 );
 };

class TBadVal: public TBadParam
 {
public:
  TBadVal(  char *p1_, char *p2_, TINIBuf* pBufINIObj = 0, char* pR = 0 ):
	 TBadParam( p1_, p2_, pBufINIObj, pR )
	  {};
 };

class TDuplicateEntry
 {
public:
  TDuplicateEntry( const char *pPar_, const char *pVal_, const char *pSect_ ):
	 pPar(pPar_), pVal(pVal_), pSect(pSect_)
	  {};

  const char* When() const;

private:
  const char *pPar, *pVal, *pSect;
 };


#endif
