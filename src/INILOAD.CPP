#include <classlib/file.h>
#include <stdio.h>
#include <conio.h>
#include <cstring.h>
#include <typeinfo.h>

#include "iniload.hpp"
#include "autorc.hpp"

#include "inidta.hpp"
#include "compile.hpp"

extern unsigned _stklen;

extern "C" void far AllocateUMB( void );
extern "C" void far FreeUMB( void );


static void near pascal Translate( char *pBuf, unsigned int uiSize );

static int near pascal CanntRead( char far* name )
 {
	printf( "CHKFLOP: Cann't read %s: '%s'\n", name, _sys_errlist[_doserrno] );
	return 1;
 }


static bool near pascal CheckInstall()
 {
	char far* pPass;
   const char far* pPs = "CheckFlop 2.0a";
	asm
	 {
		Push  ds
		Push  si
		Mov	ax,  uiASK_CODE
		Int	2Fh
		Cmp	ax, uiASK_CODE_R
		Jnz   NO_INSTALLED
		Mov	[word ptr pPass], si
		Mov	[word ptr pPass + 2], ds
		Pop   si
		Pop   ds
	 }

	if( strncmpi(pPs, pPass, strlen(pPs)) ) return false;

	return true;

NO_INSTALLED:
	asm Pop si
	asm Pop ds

	return false;
 }


static int near pascal  XAllocErr( xalloc& x )
 {
	printf( "CHKFLOP: %s\n", x.why().c_str() );
	return 1;
 }

struct TMCB
 {
	unsigned char firstSymb;
	unsigned int ownerPSP,
					 blockSize;
	unsigned char reserved[11];
 };


static TParamBlock  far* near pascal GetTSROwnMem( unsigned int uiLowLevelSize )
 {
	TParamBlock  far* pParBlc = 0;
	unsigned int uiPSP;

	asm  Mov		bx, uiLowLevelSize
	AllocateUMB();
	asm  Jc     NO_MEM
	asm  Mov    [word ptr pParBlc + 2], ax
	asm  Jmp    OK_MEM

NO_MEM:
	throw xmsg( "CHKFLOP: Not enought memory for place parameters block" );

OK_MEM:
	asm {
		 Push	ds
		 Mov	ax, uiASK_CODE
		 Int	2Fh
		 Mov  [uiPSP], ds
		 Pop  ds
	 };

	((TMCB far*)MK_FP(FP_SEG(pParBlc)-1, 0))->ownerPSP = uiPSP;
	memset( pParBlc, 0, uiLowLevelSize * 16 );

	return pParBlc;
 };

static void near pascal Translate( char *pBuf, unsigned int uiSize )
 {
	TINIBuf iniBuf( pBuf, uiSize );

	TParamBlock_Load plLoad;
	plLoad.Init( iniBuf );
	plLoad.Check();
	unsigned int uiLowLevelSize = plLoad.GetLowLevelSize();

	TParamBlock  far* pParBlc = GetTSROwnMem( uiLowLevelSize );

	plLoad.RenumRefs();
	plLoad.InitTSROwnMem( pParBlc );

	asm {
		 Push	es
		 Mov  ax, [word ptr pParBlc + 2]
		 Mov  es, ax
		 Mov  bx, [word ptr pParBlc]
		 Mov	ax, uiINI_LOAD
		 Int	2Fh
		 Pop  es
	 };
		 //FreeUMB();
		 //asm Pop  es

	/*printf( "\nAVirus:\n" );
	PrintAvir( plLoad.liAvir );

	printf( "\nDisks DOS:\n" );
	PrintDisks( plLoad.liDisksDOS );

	printf( "\nDisks Win:\n" );
	PrintDisks( plLoad.liDisksWin );

	printf( "\nDOS mem:\n" );
	PrintDOSMem( plLoad.liDOSMem );

	printf( "\n\tLSize = %u Para\n", uiLowLevelSize );*/
 }


static int MsgBadVal_Prm( TBadParam& rBP, int iValue  )
 {
	string tStr( rBP.p1, 0, abs(rBP.p1 - rBP.p2) + 1 );
	char cBuf[80] = ".\n";
	if( rBP.pRange )
	  sprintf( cBuf, ". Need %s\n", rBP.pRange );

	printf( "CHKFLOP: Bad %s '%s' in CHKFLOP.INI file%s",
			  iValue ? "value":"param", tStr.c_str(), cBuf
			);

	//printf( "%s", rBP.strErrLine.c_str() );
	return 1;
 }

int main( int argc, char* argv[], char* )
 {
	if( !CheckInstall() || argc != 2 )
	 {
		printf( "This program is used internally by CHKFLOP.COM\n" );
		return 1;
	 }

	unsigned int uiSize;
	TAutoMemoryRc<char*> ptINI;
	try {
	  ptINI = GetINIFile( argv[1], uiSize );
	  //ptINI = GetINIFile( "F:\\WORK\\CHKFLOP2\\TSR\\chkflop.ini", uiSize );
	 }
	catch( TCanntRead& )
	 {
		return CanntRead( argv[1] );
	 }
	catch( xalloc& x )//xmsg
	 {
		return XAllocErr( x );
	 }

	try {
	  Translate( ptINI(), uiSize );
	 }
	catch( TNoSection& rNoSect )
	 {
		printf( "CHKFLOP: %s\n", rNoSect.Why() );
		return 1;
	 }
	catch( xalloc& x )
	 {
		return XAllocErr( x );
	 }
	catch( xmsg& x )
	 {
		printf( x.why().c_str() );
		return 1;
	 }

	catch( TBadVal& rBP ) { return MsgBadVal_Prm( rBP, 1 ); }
	catch( TBadParam& rBP ) { return MsgBadVal_Prm( rBP, 0 ); }

	catch( TDuplicateEntry& rE )
	 {
		printf( "CHKFLOP: in CHKFLOP.INI duplicate entry: %s\n", rE.When() );
		return 1;
	 }
	catch( TAErr& aeErr )
	 {
		char cTmp[ 257 ];
		int iLen = abs(aeErr.p1-aeErr.p2) + 1;
		iLen = iLen < 256 ? iLen:256;
		strncpy( cTmp, aeErr.p1, iLen );
		cTmp[ iLen ] = 0;

		printf( "CHKFLOP: in CHKFLOP.INI invalid param: %s\n", cTmp );
		return 1;
	 }

	//printf( "Press any key...\n" );
	//getche();

	return 0;
 }

