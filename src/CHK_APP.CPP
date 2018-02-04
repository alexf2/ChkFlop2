#define _MYAPP
#include "my_defs.hpp"

#include <bwcc.h>
#include <ctl3d.h>

#include <owl\owlpch.h>
#include <owl\except.h>
#include <owl\framewin.h>

#include "chk_app.hpp"
#include "fullscr.hpp"
#include "autorc.hpp"
#include "inidta.hpp"
#include "RC\chk_app.rh"

#define SC_SETUP    0x8880
#define SC_ABOUT    0x8870
#define SC_ENBLDSBL 0x8860


extern "C" int RealIntSimulate( TRMI far*, int iBX, int iCX );

const uint16 uiTimerID = 1, uiTimerOut = 200;
static const char far* cDriverName = "chk_drv.dll";
static const char far* cTitleMain = "Autoheck flop for virus 2.0";
static const char far* cErrTtl = "CHK_FLOP Error";

static TRMI rmRmi;

void TChkFlopApp::InitMainWindow()
 {
	try {
	  MainWindow = new TMyFrame( 0, cTitleMain, 0 );
	  EnableBWCC( true );
	 }
	catch( xmsg& x )
	 {
		MessageBox( 0, x.why().c_str(),
						cErrTtl, MB_OK | MB_ICONSTOP | MB_TASKMODAL );
	 }
	catch( TDriverErrLoad& )
	 {
		MessageBox( 0, "Cann't load driver chk_drv.dll", cErrTtl,
						MB_OK | MB_ICONSTOP | MB_TASKMODAL );
	 }
	catch( TDriverErrRegister& )
	 {
		BWCCMessageBox( 0, "Cann't register on chk_drv.dll", cErrTtl,
							 MB_OK | MB_ICONSTOP | MB_TASKMODAL );
	 }
 }

/*int TChkFlopApp::Error( int errorCode )
 {
	return TApplication::Error( errorCode );
 } */

/*int TChkFlopApp::Error( xmsg& x, unsigned captionResId, unsigned promptResId )
 {
	return TApplication::Error( x, captionResId, promptResId );
 } */


TMyFrame::TMyFrame( TWindow* parent, const char far *title,
						  TWindow* clientWnd, bool shrinkToClient,
						  TModule* module
					  ): 
  TFrameWindow( parent, title, clientWnd, shrinkToClient, module ),
  hTask( GetCurrentTask() ),
  ic1( ::Module->GetInstance(), IDC_ICON_ENBL ),
  ic2( ::Module->GetInstance(), IDC_ICON_DSBL ),
  icAct1( ::Module->GetInstance(), IDC_ICON_ACT1 ),
  icAct2( ::Module->GetInstance(), IDC_ICON_ACT2 )
 {
	Attr.Style &= ~(WS_MAXIMIZEBOX | WS_THICKFRAME);

	bFlBlink = 0;
	uiTimer = 0;
	bFlReg = 0;
	bFlGlobalEnbl = true;

	hDrv = OpenDriver( cDriverName, 0, 0 );
	if( !hDrv ) throw TDriverErrLoad();

	memset( &rmRmi, 0 , sizeof(TRMI) );
	rmRmi.EAX = uiGET_CONFIG;
	RealIntSimulate( &rmRmi, 0x2F, 0 ); //ebx

	sSel = TSynthSelector( FlatAddr(rmRmi.EBX>>16, rmRmi.EBX),
								  0xFFFE );
	eControl.Ass( (TParamBlock far*)(char far*)sSel );
 }

TMyFrame::~TMyFrame()
 {
					//::MessageBox( 0, "TMyFrame Destructor", "Hello", 0 );
					//MessageBeep(-1);
	if( uiTimer )
	 {
		KillTimer( uiTimer );
		uiTimer = 0;
	 }

	if( hDrv )
	 {
		if( bFlReg )
		 {
			SendDriverMessage( hDrv, DRV_UNREGISTERTASK, (LPARAM)hTask, 0 );
			bFlReg = 0;
		 }
		CloseDriver( hDrv, 0, 0 );
		hDrv = 0;
	 }
 }

char far* TMyFrame::GetClassName()
 {
	return "CHKFLOP_WIN";
 }

void TMyFrame::GetWindowClass( WNDCLASS& wndClass )
 {
	TFrameWindow::GetWindowClass( wndClass );

	wndClass.style |= CS_DBLCLKS;
 }

void TMyFrame::SetupWindow()
 {
	TFrameWindow::SetupWindow();

	HMENU hmSys = GetSystemMenu();
	if( hmSys )
	 {
		TMenu tmSys( hmSys );
		tmSys.DeleteMenu( SC_RESTORE, MF_BYCOMMAND );
		tmSys.DeleteMenu( SC_MINIMIZE, MF_BYCOMMAND );
		tmSys.DeleteMenu( SC_MAXIMIZE, MF_BYCOMMAND );
		tmSys.DeleteMenu( SC_SIZE, MF_BYCOMMAND );
		tmSys.AppendMenu( MF_SEPARATOR );
		tmSys.AppendMenu( MF_ENABLED, SC_ABOUT, "&About" );
		tmSys.AppendMenu( MF_ENABLED, SC_SETUP, "&Setup..." );
		tmSys.AppendMenu( MF_ENABLED, SC_ENBLDSBL, "&Global enable/disable" );
	 }

	if( !(bFlReg=SendDriverMessage( hDrv, DRV_REGISTERTASK, (LPARAM)HWindow, 0 ))
	  )
	 {  //Error Unhandled
		CloseDriver( hDrv, 0, 0 );
		hDrv = 0;
		//throw TDriverErrRegister();
	 }

	hcOld = (HICON)SetClassWord( GCW_HICON, 0 );
	RefreshIcon();
 }
//TWindow

void TMyFrame::Destroy( int retVal )
 {
		 //::MessageBox( 0, "TMyFrame::Destroy", "Hello", 0 );

	if( hcOld )
	 {
		SetClassWord( GCW_HICON, (uint16)hcOld );
		hcOld = 0;
	 }

	TFrameWindow::Destroy( retVal );

 //::MessageBox( 0, "TMyFrame::Destroy OK", "Hello", 0 );
 }
																		  //sw_minimize
//WM_SHOWWINDOW  Showwindow setwindowpos WM_WINDOWPOSCHANGED
void TMyFrame::EvShowWindow( bool show, uint status )
 {
	if( !IsIconic() ) HandleMessage( WM_SYSCOMMAND, SC_ICON, 0 );
	else DefaultProcessing();
 }

void TMyFrame::RefreshIcon( bool bFlRedraw )
 {     //TIcon
	if( uiTimer )
	  SetClassWord( GCW_HICON, bFlBlink ? (WORD)(HICON)icAct1:(WORD)(HICON)icAct2 );
	else
	  SetClassWord( GCW_HICON,
						 (WORD)(HICON)((bFlGlobalEnbl == false) ? ic2:ic1) );

	if( bFlRedraw )
	  RedrawWindow( 0, 0, RDW_INVALIDATE | RDW_ERASE | RDW_FRAME );
 }

class TRCLoadError
 {
public:
  TRCLoadError( int iE, int iExtErr_ = 0 ) { iErr = iE; iExtErr = iExtErr_; }
  enum { TRcErr_FindRc = 0, TRcErr_AcessRc, TRcErr_CreTemp, TRcErr_LZ,
			TRcErr_ReadRC, TRcErr_WriteRC,
			TRcMaxErr,
			TRcMaxErrExt = 8 };

  const char* Why() const;
  const char* Ext() const;

private:
  int iErr, iExtErr;
  static char *cErr[];
  static char *cLZErrors[];
 };

char *TRCLoadError::cErr[ TRCLoadError::TRcMaxErr + 1 ] =
 {
	"EXE file dimaged: cann't find resorce",
	"Cann't acess resource",
	"Cann't create temp file",
	"",
	"Error reading resource",
   "Error writing temp file",
	"Unknown"
 };

char *TRCLoadError::cLZErrors[ TRCLoadError::TRcMaxErrExt + 1 ] =
 {
	"The handle identifying the source file was not valid.",
	"The handle identifying the destination file was not valid.",
	"The source file format was not valid.",
	"There is insufficient space for the output file.",
	"There is insufficient memory for the required buffers.",
	"The handle identifying the internal data structures is invalid.",
	"The input parameter is out of the allowable range.",
	"The source file was compressed with an unrecognized compression algorithm.",
	"Unknown"
 };

const char* TRCLoadError::Why() const
 {
	return  cErr[ (iErr >= 0 && iErr < TRcMaxErr) ? iErr:TRcMaxErr ];
 }

const char* TRCLoadError::Ext() const
 {
	if( !iExtErr ) return 0;
	int iE = abs(iExtErr) - 1;
	return cErr[ (iE >= 0 && iE < TRcMaxErr) ? iE:TRcMaxErrExt ];
 }

class TTempFile
 {
public:
  TTempFile( const char* pN, HFILE hf ):
	 cName( pN ),
	 iFlInit( 1 ),
	 handl( hf )
	{};

	int operator!() const { return handl == HFILE_ERROR; }
	HFILE operator()() const { return handl; }
	void operator~();
	void FreeRc();
  ~TTempFile();

private:
  string cName;
  HFILE handl;
  int iFlInit;
 };

void TTempFile::FreeRc()
 {
	if( handl ) { _lclose(handl); handl = 0; }
 }

void TTempFile::operator~()
 {
	FreeRc();
	if( iFlInit )
	 {
		remove( cName.c_str() ); iFlInit = 0;
	 }
 }

TTempFile::~TTempFile()
 {
	~*this;
 }


void TMyFrame::About()
 {
	HRSRC hrc = FindResource( GetApplication()->GetInstance(),
									  MAKEINTRESOURCE(ID_BTN256_ABOUT), "BITMAP256" );
	if( !hrc ) throw TRCLoadError( TRCLoadError::TRcErr_FindRc );

	//DWORD dwSizeRc = SizeofResource( GetApplication()->GetInstance(), hrc );
	TAutoFileRc<HFILE> frcRc = AccessResource( GetApplication()->GetInstance(), hrc );
	if( frcRc() < 0 ) throw TRCLoadError( TRCLoadError::TRcErr_AcessRc );

	char cBuf[ 128 ], cBuf2[ 128 ];
	GetTempFileName( 0, "exp", 0, cBuf );
	GetTempFileName( 0, "exp", 0, cBuf2 );
	//TAutoFileRc<HFILE> frcHfExp = _lcreat( cBuf, 0 );
	TTempFile frcHfExp( cBuf, _lcreat( cBuf, 0 ) );
	if( !frcHfExp() ) throw TRCLoadError( TRCLoadError::TRcErr_CreTemp );

	DWORD dwSizeRc = SizeofResource( GetApplication()->GetInstance(), hrc );
	TAutoMemoryHugeRc<BYTE*> mrcTmp( (BYTE*)farcalloc(dwSizeRc, 1) );
	if( !mrcTmp ) throw xmsg( "Low memory to load resource" );
	long resR;
	if( (resR = _hread( frcRc(), mrcTmp(), dwSizeRc)) == HFILE_ERROR ||
		 resR != dwSizeRc
	  )  throw TRCLoadError( TRCLoadError::TRcErr_ReadRC );
	frcRc.FreeRc();

	resR = _hwrite( frcHfExp(), mrcTmp(), dwSizeRc );
	if( resR == -1 || resR != dwSizeRc )
	  throw TRCLoadError( TRCLoadError::TRcErr_WriteRC );
	mrcTmp.FreeRc();

	//TAutoFileRc<HFILE> frcHfExp2 = _lcreat( cBuf2, 0 );
	TTempFile frcHfExp2( cBuf2, _lcreat( cBuf2, 0 ) );
	if( !frcHfExp2() ) throw TRCLoadError( TRCLoadError::TRcErr_CreTemp );

	resR = LZCopy( frcHfExp(),  frcHfExp2() );
	if( resR < 0 )
	  throw TRCLoadError( TRCLoadError::TRcErr_LZ, resR );
	~frcHfExp;
	frcHfExp2.FreeRc();

	TDib* ptdAbout = 0;
	try {
	  ptdAbout = new TDib( cBuf2 );
	 }
	catch(...)
	 {
		if( ptdAbout ) delete ptdAbout;
      throw;
	 }
	~frcHfExp2;

	TFullScrWindow *fswWin = 0;
	try {
	  fswWin = new TFullScrWindow( this, ptdAbout, NULL, GetModule() );
	}
	catch(...)
	 {
      if( fswWin ) delete fswWin;
		throw;
	 }
	GetApplication()->MakeWindow( fswWin );
	//fswWin->Create();
	//fswWinShow->Window();
 }

void TMyFrame::EvSysCommand( uint cmdType, TPoint& point )
 {
	switch( cmdType & 0xFFF0 )
	 {
		case SC_RESTORE:
		case SC_MAXIMIZE:
		  break;

		case SC_ABOUT:
		  try {
			 About();
			}
		  catch( TRCLoadError& rRCErr )
			{
			  BWCCMessageBox( HWindow, rRCErr.Ext() ? rRCErr.Ext():rRCErr.Why(),
									cErrTtl, MB_OK | MB_ICONSTOP );
			}
		  catch( TXOwl& tx )
			{
			  bool bFl;
			  BWCCMessageBox( HWindow, tx.ResourceIdToString(&bFl,tx.GetErrorCode(),::Module).c_str(),
									cErrTtl, MB_OK | MB_ICONSTOP );
			}
		  catch( xmsg& x )
			{
			  bool bFl;
			  BWCCMessageBox( HWindow, x.why().c_str(),
									cErrTtl, MB_OK | MB_ICONSTOP );
			}
		  break;

		case SC_SETUP:
		  SendDriverMessage( hDrv, DRV_CONFIGURE, (unsigned long)HWindow, 0 );
		  break;

		case SC_ENBLDSBL:
		  bFlGlobalEnbl = (bFlGlobalEnbl == false) ? true:false;
		  RefreshIcon( true );
		  break;

		default:
		  DefaultProcessing();
	 };
 }

LRESULT TMyFrame::EvUninstall( WPARAM wParam, LPARAM lParam )
 {
	PostQuitMessage( 0 );
	return 0;
 }


const char* TMyFrame::cErrStr[ iNumbMsg ] =
 {
	"System was out of memory, executable file was corrupt, or relocations were invalid.",
	"Unknown",
	"File was not found.",
	"Path was not found.",
	"Unknown",
	"Attempt was made to dynamically link to a task, or there was a sharing or network-protection error.",
	"Library required separate data segments for each task.",
	"Unknown",
	"There was insufficient memory to start the application.",
	"Unknown",
	"Windows version was incorrect.",
	"Executable file was invalid. Either it was not a Windows application or there was an error in the .EXE image.",
	"Application was designed for a different operating system.",
	"Application was designed for MS-DOS 4.0.",
	"Type of executable file was unknown.",
	"Attempt was made to load a real-mode application (developed for an earlier version of Windows).",
	"Attempt was made to load a second instance of an executable file containing multiple data segments that were not marked read-only.",
	"Unknown",
	"Unknown",
	"Attempt was made to load a compressed executable file. The file must be decompressed before it can be loaded.",
	"Dynamic-link library (DLL) file was invalid. One of the DLLs required to run this application was corrupt.",
	"Application requires Microsoft Windows 32-bit extensions."
 };

void TMyFrame::ErrExec( int iRes )
 {
	int iInd = (iRes >= iNumbMsg) ? 1:iRes;
	BWCCMessageBox( HWindow, cErrStr[iInd], "CHKFLOP: Error antivirus start", MB_OK | MB_ICONSTOP );
 }

int TMyFrame::ExecuteApp( string& sPath, string& sParam )
 {
	int iRes = ExecApp( HWindow, const_cast<char*>(sPath.c_str()),
						 const_cast<char*>(sParam.c_str()), &execInf,
						 GetApplication()->GetInstance() );
	if( iRes >= MAX_EXEC_ERR )
	  SetCaption( sPath.c_str() );
	return iRes;
 }

LRESULT TMyFrame::EvStartAvir( WPARAM wParam, LPARAM lParam )
 {
	if( !bFlGlobalEnbl )
	 {
		SendDriverMessage( hDrv, DRV_ENDTEST, 0, 0 );
		return 0;
	 }

	SetActiveWindow();

	eControl.Start( lParam );
	string sPath, sParam;
	eControl.Next( sPath, sParam );
	if( !sPath.length() )
	 {
		SendDriverMessage( hDrv, DRV_ENDTEST, 0, 0 );
		return 0;
	 }
	unsigned int iRes = ExecuteApp( sPath, sParam );

	/*ExecApp( HWindow, "D:\\AIDS\\aidstest.exe", "d:*.*", &execInf,
										  GetApplication()->GetInstance() );*/
	if( iRes < MAX_EXEC_ERR )
	 {
		SendDriverMessage( hDrv, DRV_ENDTEST, 0, 0 );
		ErrExec( iRes );
	 }
	else
	 {
		bFlBlink = 0;
		uiTimer = SetTimer( uiTimerID, uiTimerOut );
	 }

	return 0;
 }


void TMyFrame::EvTimer( uint timerId )
 {
	static int iSem = 0;

	if( iSem )
	 {
		TWindow::EvTimer( timerId );
		return;
	 }

	iSem = 1;

	if( timerId == uiTimerID && !IsAppRunning(&execInf) )
	 {
		string sPath, sParam;
		eControl.Next( sPath, sParam );
		if( sPath.length() )
		 {
			unsigned int iRes = ExecuteApp( sPath, sParam );
			if( iRes < MAX_EXEC_ERR ) ErrExec( iRes );
			else
			 {
				TWindow::EvTimer( timerId );
				iSem = 0;
				return;
			 }
		 }

		KillTimer( uiTimer ); uiTimer = 0;
		SendDriverMessage( hDrv, DRV_ENDTEST, 0, 0 );
		SetCaption( cTitleMain );
		RefreshIcon( true );
	 }
	else
	 {
		RefreshIcon( true );
		bFlBlink = !bFlBlink;
	 }

	TWindow::EvTimer( timerId );
	iSem = 0;
 }

bool TMyFrame::CanClose()
 {
	if( uiTimer ) BWCCMessageBox( HWindow, "Antivirus still active !",
											"CHK_FLOP cann't close", MB_OK | MB_ICONSTOP );
	return  !uiTimer;
 }

DEFINE_RESPONSE_TABLE1( TMyFrame, TFrameWindow )
  EV_WM_SHOWWINDOW,
  EV_WM_SYSCOMMAND,
  EV_WM_TIMER,
  EV_WM_SYSCOLORCHANGE,
  EV_MESSAGE( WM_UNINSTALL, EvUninstall ),
  EV_MESSAGE( WM_START_AVIR, EvStartAvir ),
END_RESPONSE_TABLE;


void TMyFrame::EvSysColorChange()
 {
	//Ctl3dColorChange();
	TFrameWindow::EvSysColorChange();
 }

