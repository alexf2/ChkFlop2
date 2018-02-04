#define _MYAPP
#include "my_defs.hpp"

#include <owl\owlpch.h>
#include <owl\applicat.h>
#include <owl\framewin.h>

#include <owl\gdiobjec.h>
#include <stdio.h>
#include <bwcc.h>

#include "chk_drv.hpp"
#include "ic.hpp"
#include "chk_cfg.hpp"
#include "synth.hpp"
#include "chk_inst.hpp"
#include "inidta.hpp"

extern "C" int RealIntSimulate( TRMI far*, int iBX, int iCX );
extern "C" void MyCallBack( void );
extern "C" void hWndChk( void );


static TRMI rmRmi;
static uint32 ulCallAddr = 0;
static uint16 *pHWndChk = 0;

const char far* pChkErrMsg = "Chk_drv error loading"; 

void near pascal UnRegisterTask();

static void near pascal CleanUp()
 {
	if( pHWndChk )
	 {
		UnRegisterTask();
		FreeSelector( FP_SEG(pHWndChk) ); pHWndChk = 0;
	 }

	if( ulCallAddr )
	 {
		asm mov  ax, 0304h
		asm mov 	cx, WORD PTR ulCallAddr+2
		asm mov 	dx, WORD PTR ulCallAddr
		asm int 	31h
		ulCallAddr = 0;
	 }
	if( Module ) { delete Module; Module = 0; }
 }


int CALLBACK LibMain( HINSTANCE hModule, WORD wDataSegm, WORD wHeapSize, LPSTR lpCmd )
 {
			//MessageBox(0, "LibMain", "Hello", 0 );
	if( !CheckInstall() )
	 {
		BWCCMessageBox( 0, "CHKFLOP.COM TSR program must be installed before start WINDOWS",
		                pChkErrMsg, MB_OK | MB_ICONSTOP | MB_TASKMODAL );
				 //MessageBox(0, "Check1 abort", "Hello", 0 );
		return false;
	 }

	::Module = new TModule( 0, hModule );

	asm push	ds
	asm push	es
	asm push	si
	asm push	di

	asm mov  ax, 0303h               // Allocate real mode callback
													// address
	asm push ds
	asm push cs
	asm pop  ds
	asm mov  si, OFFSET MyCallBack
	asm pop  es
	asm mov  di, OFFSET rmRmi       // Offset of real mode call
													//   structure, used for DPMI
													//   translation services
													//   (see Chapter 11 of the
													//   INTEL DPMI Spec. v 0.9)
	asm int  31h                     // Call DPMI
	asm pop  di
	asm pop  si
	asm pop  es
	asm pop  ds
	asm jc   CB_Error                // If carry set, call failed
	asm jmp  Set_CB_Addr

CB_Error:
	BWCCMessageBox( 0, "Cann't callback allocate",
							 pChkErrMsg, MB_OK | MB_ICONSTOP | MB_TASKMODAL );
	CleanUp();
			//MessageBox(0, "Check2 abort", "Hello", 0 );
	return false;
Set_CB_Addr:
		asm mov  WORD PTR ulCallAddr+2, cx   // Store SEG:OFF of callback
		asm mov  WORD PTR ulCallAddr, dx     //   address in local variable

	uint16 uiSel = AllocSelector( 0 );
	uiSel = PrestoChangoSelector( _CS, uiSel );
	pHWndChk = (uint16 *)MK_FP( uiSel, FP_OFF(hWndChk) );


			  //MessageBox(0, "LibMain OK", "Hello", 0 );
	return true;
 }


int CALLBACK  WEP( int iSysExit )
 {
	CleanUp();
					  //MessageBox(0, "WEP", "Hello", 0 );
	return 1;
 }

static void near pascal RegisterTask( uint16 uiTask )
 {
	if( !pHWndChk ) return;
	*pHWndChk = uiTask;

	memset( &rmRmi, 0 , sizeof(TRMI) );
	rmRmi.EAX = uiREGISTER_CALLBACK;
	rmRmi.EBX = FP_SEG( ulCallAddr );
	rmRmi.ECX = FP_OFF( ulCallAddr );
	RealIntSimulate( &rmRmi, 0x2F, 0 );
 }

static void near pascal UnRegisterTask()
 {
	*pHWndChk = 0;
	memset( &rmRmi, 0 , sizeof(TRMI) );
	rmRmi.EAX = uiUNREGISTER_CALLBACK;
	RealIntSimulate( &rmRmi, 0x2F, 0 );
 }

static void near pascal EndOfTest()
 {
	memset( &rmRmi, 0 , sizeof(TRMI) );
	rmRmi.EAX = uiEND_TEST;
	RealIntSimulate( &rmRmi, 0x2F, 0 );
 }


static void near pascal IssueCfg( LPARAM lParam1 )
 {
	try {
	  TModule mChk( "CHK_CFG.DLL", true );

	  void far cdecl (*ChkCfg)(unsigned int);
	  (FARPROC)ChkCfg = mChk.GetProcAddress( "ChkCfg" );
	  if( !ChkCfg ) throw xmsg( "Cann't initialize CHK_CFG.DLL" );
	  ChkCfg( (unsigned int)(lParam1) );
	 }
	catch( xmsg& x )
	 {
		MessageBox( (HWND)lParam1, x.why().c_str(),
						pChkErrMsg, MB_OK );
		return;
	 }
	catch( ... )
	 {
		HandleGlobalException( xmsg(string()),
					 "Abnormal Termination, uncaught ...", 0 );
	 }
 }


extern "C" LRESULT CALLBACK _export DriverProc ( DWORD   dwDriverIdentifier,
																 HDRVR   hDriver,
																 UINT    wMessage,
																 LPARAM  lParam1,
																 LPARAM  lParam2 )
 {
	DWORD dwRes = 0L;

	switch( wMessage )
	 {
	  case DRV_LOAD:
		 //MessageBox( 0, "DRV_LOAD", "Hello", 0 );
		 dwRes = 1L;
		 break;

	  case DRV_CLOSE:
		 //MessageBox( 0, "DRV_CLOSE", "Hello", 0 );
		 dwRes = 1L;
		 break;
	  case DRV_OPEN:
		 //MessageBox( 0, "DRV_OPEN", "Hello", 0 );
		 dwRes = 1L;
		 break;

	  case DRV_FREE:
		 //MessageBox( 0, "DRV_FREE", "Hello", 0 );
		 dwRes = 1L;
		 break;

	  case DRV_DISABLE:
	  case DRV_ENABLE:
	  case DRV_REMOVE:
	  case DRV_INSTALL:
	  case DRV_QUERYCONFIGURE:
		dwRes = 1L;
		break;

	  case DRV_CONFIGURE:
		 IssueCfg( lParam1 );
		 dwRes = 1L;
		 break;

	  case DRV_REGISTERTASK:
		 if( pHWndChk && *pHWndChk )
		  {
			 BWCCMessageBox( 0, "CHK_APP already registred",
								  pChkErrMsg, MB_OK | MB_ICONSTOP | MB_TASKMODAL );
			 return 0;
		  }
		 dwRes = 1L;
		 RegisterTask( (uint16)lParam1 );
		 //PostMessage( (HWND)lParam1, WM_START_AVIR, 0, 0 );
		 //BOOL PostMessage(hwnd, uMsg, wParam, lParam)
		 break;

	  case DRV_ENDTEST:
		 EndOfTest();
		 dwRes = 1L;
		 break;

	  case DRV_UNREGISTERTASK:
		 dwRes = 1L;
		 UnRegisterTask();
		 break;

	  default:
		return DefDriverProc( dwDriverIdentifier, hDriver,	wMessage, lParam1, lParam2 );
	}

	return dwRes;
 }


