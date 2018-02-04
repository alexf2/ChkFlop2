#define _MYDLL
#include "my_defs.hpp"

#include <bwcc.h>
#include <stdio.h>
//#include <typeinfo.h>
#include <stdlib.h>

#include "chk_cfg.hpp"
#include "opt_dlg.hpp"
#include "tbwcc.hpp"
#include "autorc.hpp"
#include "RC\chk_cfg.rh"
#include "synth.hpp"
#include "inidta.hpp"
#include "iniload.hpp"

extern "C" int RealIntSimulate( TRMI far*, int iBX, int iCX );
static const char far* cTtl = "®AlexCorp.  Check flop 2.0a setup";
static const char far* cErrTtl = "CHKFLOP: error";


static int near pascal Unhandled()
 {
	return HandleGlobalException( xmsg(string()),
	  "Unhandled exception.", 0 );
 }

static int near pascal  XAllocErr( xalloc& x )
 {
	MessageBox( 0, x.why().c_str(), cErrTtl,
				   MB_OK | MB_ICONSTOP | MB_TASKMODAL );
	return 1;
 }

static int near pascal  XMsgErr( xmsg& x )
 {
	MessageBox( 0, x.why().c_str(), cErrTtl,
  			      MB_OK | MB_ICONSTOP | MB_TASKMODAL );
	return 1;
 }

static int MsgBadVal_Prm( TBadParam& rBP, int iValue  )
 {
	string tStr( rBP.p1, 0, abs(rBP.p1 - rBP.p2) + 1 );
	char cBuf[80] = ".";
	if( rBP.pRange )
	  sprintf( cBuf, ". Need %s", rBP.pRange );

	char cErrB[300];
	sprintf( cErrB, "Bad %s '%s' in CHKFLOP.INI file%s",
			  iValue ? "value":"param", tStr.c_str(), cBuf
			);

	MessageBox( 0, cErrB, cErrTtl,
					MB_OK | MB_ICONSTOP | MB_TASKMODAL );

	return 1;
 }

static void near pascal GetINIPath( char *pName )
 {
	TRMI rmi;

	memset( &rmi, 0 , sizeof(TRMI) );
	rmi.EAX = uiGET_STA_DIR;
	RealIntSimulate( &rmi, 0x2F, 0 );

	TSynthSelector sSel( FlatAddr(rmi.ES, rmi.EBX), 0xFFFF );
	strcpy( pName, (char far*)sSel );

	char dr[_MAX_DRIVE+1], dir[_MAX_DIR+1];
	_splitpath( pName, dr, dir, 0, 0 );
	_makepath( pName, dr, dir, "CHKFLOP", ".INI" );
 }

static bool near pascal Save( TParamBlock_Load& plLoad, char far *cName )
 {
	try {
	  plLoad.Save( cName );
	 }
	catch( xmsg& x )
	 {
		MessageBox( 0, x.why().c_str(), "CHK_FLOP Error",
						MB_OK | MB_ICONSTOP | MB_TASKMODAL );
		return false;
	 }
	catch( ... )
	 {
		Unhandled();
		return false;
	 }

	return true;
 }

static char far* near pascal CreateNewBuf( unsigned int& uiSize )
 {
	static const char* cBuf =
	 "[AVirus]\n"
	 "  1 = aidstest.exe #:\*.* /f/s\n"

	 "[DisksDOS]\n"
	 "  A = 1\n"

	 "[DisksWin]\n"
	 "  A = 1\n"

	 "[DOSMemory]\n"
	 "  1 = 19200\n"

	 "[Options]\n"
	 "  CheckSerial = Yes\n"
	 "  SizeOfSerialQueue = 3\n"
	 "  RetryNumberSec1ReadDOS = 3\n"
	 "  RetryNumberSec1ReadWin = 5\n";

	char *pTmp = new char[ (uiSize=strlen(cBuf)+1) + 1 ];
	strcpy( pTmp, cBuf );

	return pTmp;
 }

//void far PASCAL (*ChkCfg)(unsigned int);
extern "C" void _export far pascal ChkCfg( unsigned int uiHWND )
 {                         //TWindow
	char cErrMsg[300], cININame[130];
	bool bFlDirty = false;

	TBWCC bwccLoad;
	if( !bwccLoad.iStatus ) return;

	TWindow* parentAlias = GetWindowPtr( (HWND)uiHWND );
	if (!parentAlias)
	  parentAlias = new TWindow( (HWND)uiHWND, ::Module );

	GetINIPath( cININame );

	unsigned int uiSize;
	TAutoMemoryRc<char*> ptINI;
	try {
	  ptINI = GetINIFile( cININame, uiSize );
	 }
	catch( TCanntRead& )
	 {
		sprintf( cErrMsg, "CHKFLOP: Cann't read %s: '%s'. Create new ?", "CHKFLOP.INI",
					_sys_errlist[_doserrno] );
		if( MessageBox( parentAlias->HWindow, cErrMsg, "Message",
							 MB_YESNO | MB_ICONEXCLAMATION) !=
			 IDYES
		  ) return;
		try {
		  ptINI = CreateNewBuf( uiSize );
		  bFlDirty = true;
		 }
		catch(...) { throw; }
	 }
	catch( xalloc& x )//xmsg
	 {
		XAllocErr( x );
		return;
	 }

	TAutoMemoryRc<TINIBuf*> arcIniBuf;
	TParamBlock_Load plLoad;

	try
	 {
		arcIniBuf = new TINIBuf( ptINI(), uiSize );

		plLoad.Init( *arcIniBuf() );
		plLoad.Check();
		plLoad.RenumRefs();
	 }
	catch( TNoSection& rNoSect )
	 {
		MessageBox( parentAlias->HWindow, rNoSect.Why(), "Error",
						MB_OK | MB_ICONSTOP );
		return;
	 }
	catch( xalloc& x )
	 {
		XAllocErr( x );
		return;
	 }
	catch( xmsg& x )
	 {
		XMsgErr( x );
		return;
	 }
	catch( TBadVal& rBP ) { MsgBadVal_Prm( rBP, 1 ); return; }
	catch( TBadParam& rBP ) { MsgBadVal_Prm( rBP, 0 ); return; }
	catch( TDuplicateEntry& rE )
	 {
		sprintf( cErrMsg, "In CHKFLOP.INI duplicate entry: %s", rE.When() );
		MessageBox( parentAlias->HWindow, cErrMsg, cErrTtl,
						MB_OK | MB_ICONSTOP );
		return;
	 }
	catch( TAErr& aeErr )
	 {
		char cTmp[ 257 ];
		int iLen = abs( aeErr.p1-aeErr.p2 ) + 1;
		iLen = iLen < 256 ? iLen:256;
		strncpy( cTmp, aeErr.p1, iLen );
		cTmp[ iLen ] = 0;

		sprintf( cErrMsg, "In CHKFLOP.INI invalid param: %s", cTmp );
		MessageBox( parentAlias->HWindow, cErrMsg, cErrTtl,
						MB_OK | MB_ICONSTOP );
		return;
	 }
	catch( ... )
	 {
		Unhandled();
		return;
	 }

	int iRes;

	try {
	  TOptionsDialog optDlg( plLoad, cTtl, parentAlias, DLG_OPTIONS,
									 ::Module );
	  iRes = optDlg.Execute();
	  if( bFlDirty == false ) bFlDirty = optDlg.IsDirty();
	 }
	catch( TXOwl& tx )
	 {
		bool bFl;
		MessageBox( parentAlias->HWindow,
						tx.ResourceIdToString(&bFl,tx.GetErrorCode(),
						::Module).c_str(),
						cErrTtl, MB_OK | MB_ICONSTOP );
		return;
	 }
	catch( xmsg& x )
	 {
		XMsgErr( x );
		return;
	 }
	catch( ... )
	 {
		Unhandled();
		return;
	 }

	if( iRes == IDOK && bFlDirty == true )
	  if( Save(plLoad, cININame) )
		 if( MessageBox( (HWND)uiHWND, "Changes take effect only after reboot DOS.\nReboot lets ?",
							  "Message",
							  MB_YESNO | MB_ICONEXCLAMATION ) == IDYES
			)
			ExitWindows( EW_REBOOTSYSTEM, 0 );
 }


