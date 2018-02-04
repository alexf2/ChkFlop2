#if !defined(_INI_DTA_)
#define _INI_DTA_

#include <cstring.h>
#include <classlib\dlistimp.h>

#include "tauto.hpp"
#include "iniload.hpp"

struct TDriveEntry
 {
	unsigned char ucDriveInt13;
	unsigned char ucDriveDOS;     //0xFF - drive absent
	uint16 uiIndex;
 };

struct TDrivesList
 {
	int iNumbEntries;
	TDriveEntry  deEntryes[1];
 };


struct TAVirusEntry
 {
	uint16 uiMem;
	char  near* pStrCom;    //str, 0, $
	char  near* pStrParam; // SIZE, str, 0Dh, 0, $
								  //  1b ,    , 1b
 };

struct TAVirusList
 {
	uint16 ucMemMax;
	int iNumbAVirus;
	TAVirusEntry avEntryes[1];
 };

struct TOptions
 {
	unsigned char flCheckSerial;
	unsigned int  iSizeOfSerialQueue,
					  iRetryNumberSec1ReadDOS,
					  iRetryNumberSec1ReadWin;

 };

struct TParamBlock
 {
	TOptions     opt;
	TAVirusList near*  pAvL;
	TDrivesList near* pDrLDOS;
	TDrivesList near* pDrLWin;
 };


template<class T> class TMySDoubleListImp: public TSDoubleListImp<T>
 {
public:
  T* Find2( const T& );
 }

template<class T> int Cond_DblMy( const T& rD, void* pD )
 {
	return rD == *(T*)pD;
 }

template<class T> T* TMySDoubleListImp<T>::Find2( const T& rT )
 {
	return FirstThat( Cond_DblMy, (void *)&rT );
 }


typedef TMySDoubleListImp<int> TListOf_Int;

class TAVir;

class TDescriptor
 {
public:
  TDescriptor( int iId_ ): iId( iId_ )
	{
	}

  TDescriptor( TDescriptor& rD )
	{
	  iId = rD.iId;
	}

  TDescriptor& operator=( const TDescriptor& rD )
	{
	  iId = rD.iId;
	  return *this;
	}

  operator int() { return iId; }

  int operator==( const TDescriptor& avD ) const
	{
	  return iId == avD.iId;
	}
  int operator==( const TDescriptor* avD ) const
	{
	  return operator==( *avD );
	}

  int operator<( const TDescriptor& avD ) const
	{
	  return  iId < avD.iId;
	}
  unsigned HashValue() const { return 0; }


  void ChgID( int iNewID ) { iId = iNewID; }

protected:
  int iId;
 };

class TAVDescr: public TDescriptor
 {
public:
  TAVDescr():TDescriptor( -1 ) { /*iDrive = -1;*/ }
  TAVDescr( int iDr ):TDescriptor( iDr ) { /*iDrive = iDr;*/ }

  void operator+=( int iAvir );
  void operator-=( int iAvir );
  int operator[]( int iInd );

  void Ass( int iDr ) { iId = iDr; }
  TListOf_Int& GetChain() { return liAvir; }
private:
  //int iDrive;//0=A  - as int 13h
  TListOf_Int liAvir;
 };


class TAVir:public TDescriptor
 {

friend TJmpFunc_ Path_;
friend TJmpFunc_ Lbl_;
friend TJmpFunc_ None_;
friend TJmpFunc_ Mask_;
friend TJmpFunc_ Prefix_;
friend TJmpFunc_ PostFix_;

public:
  TAVir():TDescriptor( -1 ) {}
  TAVir( const char far* pStr, int id ):
	 TDescriptor( id )
	{
	  Split( pStr );
	}

  void GetFullStr( char far* pDst, int iMax ) const;
  void GetExecParams( char far* pStr, char far* pParams ) const;

  const char far* Path() const { return strPath.c_str(); }
  const char far* Prefix() const { return strPrefix.c_str(); }
  const char far* Postfix() const { return strPostfix.c_str(); }
  const char far* Mask() const { return strMask.c_str(); }

  TAVir& operator=( const char far* pcStr ) throw( TInvalidChar, TBadFormat )
	{
	  Split( pcStr );
	  return *this;
	}
  TAVir& operator=( const TAVir& av )
	{
	  //iId = av.iId;
	  TDescriptor::operator=( av );

	  strPath    = av.strPath;
	  strPrefix  = av.strPrefix;
	  strPostfix = av.strPostfix;
	  strMask    = av.strMask;

	  return *this;
	}

protected:
  string  strPath;
  string  strPrefix;
  string  strPostfix;
  string  strMask;

private:
  //int iId;

  void Split( const char far* );
 };

const uint16
				 uiINI_LOAD            = 0xFF00,
				 uiGET_CONFIG          = 0xFF01,
				 uiGET_STA_DIR         = 0xFF02,
				 uiEND_TEST            = 0xFF03,
				 uiUNREGISTER_CALLBACK = 0xFF04,
				 uiREGISTER_CALLBACK   = 0xFF05,
				 uiASK_CODE_R          = 0x06FF,
				 uiASK_CODE            = 0xFF06;



struct TMemDescr: public TDescriptor
 {
	TMemDescr(): TDescriptor(1) { uiMem = 50; }
	TMemDescr( int iInd, unsigned int uiM ):
	  TDescriptor( iInd )
	  {
		 uiMem = uiM;
	  }																																																		int operator[]( int iInd );

	//int iIndex;
	unsigned int uiMem;
 };


template<class T> class TIDblListImpDel: public TISDoubleListImp<T>
 {
public:
  ~TIDblListImpDel() { Flush(1); }
  T* Find2( const T* );
 }
				//typedef int ( *CondFunc)(const T &, void *);
template<class T> int Cond_DblDel( const T& rD, void* pD )
 {
	return rD == *(T*)pD;
 }

template<class T> T* TIDblListImpDel<T>::Find2( const T* pT )
 {
	return FirstThat( Cond_DblDel, (void *)pT );
 }


struct TParamBlock_Load
 {
	TOptions opt;
	TIDblListImpDel<TMemDescr> liDOSMem;
	TIDblListImpDel<TAVDescr> liDisksDOS;
	TIDblListImpDel<TAVDescr> liDisksWin;
	TIDblListImpDel<TAVir> liAvir;

	void Init( TINIBuf& iniBuf );
	void Check() throw( xmsg );
	unsigned int GetLowLevelSize();

	void InitTSROwnMem( TParamBlock  far* pParBlc );
	void RenumRefs();

	int GetFreeIndex();
	void Save( char *fName );

private:
  unsigned long SzAvirList();
  unsigned long SzDriveList( TIDblListImpDel<TAVDescr>& );
  int CalcDriveDescrs( TIDblListImpDel<TAVDescr>& liDriv );
  void FillDiskEntrys( TDriveEntry far*, TIDblListImpDel<TAVDescr>&, TAVirusEntry far* );
  void RenumRefsDrv( TIDblListImpDel<TAVDescr>&, int, int );
 };


#endif

