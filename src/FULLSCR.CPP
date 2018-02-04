#include <windowsx.h>
#include "fullscr.hpp"
#include "RC\chk_app.rh"
//#include <stdio.h>

//extern HINSTANCE hInstRC;
//extern PTModule pMRC;


int TSysPaletteSave::iIndexPal[ iNumbEnt ] =
 {
   COLOR_ACTIVEBORDER,
   COLOR_ACTIVECAPTION,
   COLOR_APPWORKSPACE,
   COLOR_BACKGROUND,
   COLOR_BTNFACE,
   COLOR_BTNHIGHLIGHT,
   COLOR_BTNSHADOW,
   COLOR_BTNTEXT,
   COLOR_CAPTIONTEXT,
   COLOR_GRAYTEXT,
   COLOR_HIGHLIGHT,
   COLOR_HIGHLIGHTTEXT,
	COLOR_INACTIVEBORDER,
   COLOR_INACTIVECAPTION,
   COLOR_INACTIVECAPTIONTEXT,
   COLOR_MENU,
   COLOR_MENUTEXT,
   COLOR_SCROLLBAR,
   COLOR_WINDOW,
   COLOR_WINDOWFRAME,
   COLOR_WINDOWTEXT
 };

COLORREF TSysPaletteSave::rgb[ iNumbEnt ];


TSysPaletteSave::~TSysPaletteSave()
 {
   ~*this;
 }

void TSysPaletteSave::operator()( HDC hdc_ )
 {
   Reset();   

	iNumColors = GetDeviceCaps( hdc, NUMCOLORS );
   if( !hdc_ || !iNumColors || iNumColors > 20 ) return;
   hdc = hdc_;

   for( int i = 0; i < iNumbEnt; i++ )
     TSysPaletteSave::rgb[ i ] = GetSysColor( TSysPaletteSave::iIndexPal[i] );
 }

void TSysPaletteSave::operator~()
 {
   if( hdc )
     SetSysColors( iNumbEnt , TSysPaletteSave::iIndexPal,
		   TSysPaletteSave::rgb );
   else return;

   Reset();
 }

void TSysPaletteSave::Reset()
 {
	hdc = NULL;
 }

int TFullScrWindow::iMouseX, TFullScrWindow::iMouseY;

TFullScrWindow::TFullScrWindow( TWindow* parent, TDib*  pbmpDIB_, HPALETTE hpalAlias_, TModule* AModule ):
  TWindow( parent, 0, AModule ),
  pal( *pbmpDIB_ )//TPalette
 {
	bFlOldDisable = FALSE;
	hpalAlias = hpalAlias_;
	Attr.Style = WS_BORDER | WS_MAXIMIZE | WS_VISIBLE | WS_POPUP;
	pbmpDIB = pbmpDIB_;
	iBmpWidth = pbmpDIB->Width(); //TDIB
	iBmpHigh = pbmpDIB->Height();
	hcHand = NULL;
	hdc = NULL;

	iStaOrgX = iStaOrgY = 0;
	bFlTrack = FALSE;

	_fsetmem( &r, sizeof(r), 0 );

	pBmp = 0;
 }
void TFullScrWindow::Destroy( int retVal )
 {
	TApplication* pApp = GetApplication();
	if( pApp && pApp->MainWindow )
	  ::EnableWindow( pApp->MainWindow->HWindow, bFlOldDisable ? FALSE:TRUE );

	::SetSystemPaletteUse( hdc, SYSPAL_STATIC );
	~spsSysPalette;

	TWindow::Destroy( retVal );

	if( Parent )
	  ::PostMessage( Parent->HWindow, WM_USER_FULLSCRDONE, 0, 0 );
 }

TFullScrWindow::~TFullScrWindow()
 {
	if( hcHand ) { DestroyCursor( hcHand ); hcHand = NULL; }
	if( pBmp ) { delete pBmp; pBmp = 0; }
	if( pbmpDIB ) { delete pbmpDIB; pbmpDIB = 0; }
 }

LPSTR TFullScrWindow::GetClassName()
 {
	return "CHK_APP:FULLSCRWINDOW";
 }

void TFullScrWindow::GetWindowClass( WNDCLASS & AWndClass )
 {
	TWindow::GetWindowClass( AWndClass );

	AWndClass.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
	AWndClass.style |= CS_SAVEBITS | CS_OWNDC;
 }

BOOL TFullScrWindow::Create()
 {
	if( !TWindow::Create() ) return FALSE;

	TApplication* pApp = GetApplication();
	if( pApp && pApp->MainWindow )
	  bFlOldDisable = ::EnableWindow( pApp->MainWindow->HWindow, FALSE );

	hdc = GetDC( HWindow );
	spsSysPalette( hdc );
	SetSystemPaletteUse( hdc, SYSPAL_NOSTATIC );

	::GetClientRect( HWindow, &r );
	iBmpViewWidth = r.right; iBmpViewHigh = r.bottom;
	hcHand = LoadCursor( GetModule()->GetInstance(), MAKEINTRESOURCE(CSR_OT_Hand) );

	if( iBmpWidth < r.right ) iStaOrgX = (r.right - iBmpWidth) / 2;
	if( iBmpHigh < r.bottom ) iStaOrgY = (r.bottom - iBmpHigh) / 2;
	SetViewportOrg( hdc, iStaOrgX, iStaOrgY );

	::InvalidateRect( HWindow, NULL, TRUE );

	return TRUE;
 }

bool TFullScrWindow::EvEraseBkgnd( HDC hdc )
 {
	if( iBmpWidth >= r.right && iBmpHigh >= r.bottom ) return 1;
	return (bool)DefaultProcessing();
 }

void near pascal TFullScrWindow::DrawBitmapDIB( HDC Hdc, int X, int Y )
 {
	HBITMAP Hbm, HbmOld;
	//HBITMAP HbmBitmap;
	HDC     HdcMem;
	BITMAP  Bm;
	POINT   PtSize, PtOrg;


	if( !pBmp )
	 {
		HCURSOR hcOld = ::SetCursor( LoadCursor(0, IDC_WAIT) );
		pBmp = new TBitmap( Hdc, *pbmpDIB );
		::SetCursor( hcOld );
	 }

	HBITMAP HbmBitmap = (HBITMAP)*pBmp;


	if( !(HdcMem = CreateCompatibleDC( Hdc )) )
	 {
		//DeleteObject( HbmBitmap );
		return;
	 }

	if( (HbmOld = (HBITMAP)SelectObject( HdcMem, HbmBitmap )) )
	 {
		SetMapMode( HdcMem, GetMapMode(Hdc) );
		SetViewportOrg( HdcMem, LOWORD( GetViewportOrg(Hdc) ),
							 HIWORD( GetViewportOrg(Hdc) ) );
		SetWindowOrg( HdcMem, LOWORD( GetWindowOrg(Hdc) ),
						  HIWORD( GetWindowOrg(Hdc) ) );
		//SetMapMode( HdcMem, GetMapMode(Hdc) );
		GetObject( HbmBitmap, sizeof(BITMAP), (LPSTR)&Bm );

		PtSize.x = Bm.bmWidth;
		PtSize.y = Bm.bmHeight;
		//DPtoLP( Hdc, &PtSize, 1 );

		PtOrg.x = PtOrg.y = 0;
		DPtoLP( HdcMem, &PtOrg, 1 );

		BitBlt( Hdc, X, Y, PtSize.x, PtSize.y, HdcMem, PtOrg.x, PtOrg.y, SRCCOPY );

		SelectObject( HdcMem, HbmOld );
	 }

	DeleteDC( HdcMem );
	//DeleteObject( HbmBitmap );
 }


void near pascal TFullScrWindow::DrawBitmapDIBWithPalette( HDC Hdc, HPALETTE hpalAlias, int X, int Y )
 {
	HPALETTE hpal = hpalAlias ? hpalAlias:(HPALETTE)pal;//TPalette

	if( hpal )
	 {
		HPALETTE HpalOld = SelectPalette( Hdc, hpal, FALSE );
		//RealizePalette( Hdc );

		DrawBitmapDIB( Hdc, 0, 0 );

		SelectPalette( Hdc, HpalOld, FALSE );
	 }
	else DrawBitmapDIB( Hdc, 0, 0 );
 }


void TFullScrWindow::Paint( TDC& dc, bool erase, TRect& rect )
 {
	if( pbmpDIB )
	  DrawBitmapDIBWithPalette( dc, hpalAlias, 0, 0 );
 }

void TFullScrWindow::PrepareKey( uint16 uiKey )
 {
	if( pbmpDIB )
	 {
		int iPoW = iStaOrgX,
	  iPoH = iStaOrgY;
		int iDx = iBmpWidth - iBmpViewWidth,
	  iDy = iBmpHigh - iBmpViewHigh;
		int iVStep = double(iBmpViewHigh) * 0.2,
	  iHStep = double(iBmpViewWidth) * 0.2;

		switch( uiKey )
		 {
			case VK_UP:
			  iPoH++;
		break;

			case VK_DOWN:
			  iPoH--;
		break;

			case VK_LEFT:
			  iPoW++;
		break;

	 case VK_RIGHT:
		iPoW--;
		break;

	 case VK_HOME:
		if( GetKeyState(VK_SHIFT) & 0xFF00 )
		  iPoH = 0;
			  else
		  iPoW = 0;
		break;

	 case VK_END:
		if( GetKeyState(VK_SHIFT) & 0xFF00 )
		  iPoH = -iDy;
			  else
		  iPoW = -iDx;
		break;

	 case VK_PRIOR:
		if( GetKeyState(VK_SHIFT) & 0xFF00 )
		  iPoW += iHStep;
			  else
		  iPoH += iVStep;
		break;

	 case VK_NEXT:
		if( GetKeyState(VK_SHIFT) & 0xFF00 )
		  iPoW -= iHStep;
			  else
		  iPoH -= iVStep;
		break;

	 default:
			  int cod = uiKey;
		//TWindow::DefWndProc( msg );
		DefaultProcessing();
		if( cod != VK_SHIFT )
		  //CloseWindow();
				 ::PostMessage( HWindow, WM_CLOSE, 0, 0 );
			  return;
		 }


		if( iDx > 0 || iDy > 0 )
		 {
			if( iPoW > 0 ) iPoW = 0;
			else if( iPoW < -iDx ) iPoW = -iDx;
			if( iPoH > 0 ) iPoH = 0;
	 else if( iPoH < -iDy ) iPoH = -iDy;

	 if( iPoW != iStaOrgX || iPoH != iStaOrgY )
	  {
		 iStaOrgX = (iDx> 0) ? iPoW:0;
		 iStaOrgY = (iDy > 0) ? iPoH:0;
		 SetViewportOrg( hdc, iStaOrgX, iStaOrgY );

		 ::InvalidateRect( HWindow, NULL, TRUE );
	  }
		 }
	 }
	else ::PostMessage( HWindow, WM_CLOSE, 0, 0 );//CloseWindow();

	DefaultProcessing();
 }


void TFullScrWindow::EvKeyDown( uint key, uint repeatCount, uint flags )
 {
	PrepareKey( key );
 }

void TFullScrWindow::EvSysKeyDown( uint key, uint repeatCount, uint flags )
 {
	PrepareKey( key );
 }


void TFullScrWindow::EvLButtonDown( uint modKeys, TPoint& point )
 {
	TWindow::EvLButtonDown( modKeys, point );
	::SetCursor( hcHand );
	bFlTrack = TRUE;
 }

bool TFullScrWindow::EvSetCursor( HWND hWndCursor, uint hitTest, uint mouseMsg )
 {
	if( bFlTrack == TRUE )
	 {
		::SetCursor( hcHand );
		return 1;
	 }
	else return TWindow::EvSetCursor( hWndCursor, hitTest, mouseMsg );
 }

void TFullScrWindow::EvLButtonUp( uint modKeys, TPoint& point )
 {
	TWindow::EvLButtonUp( modKeys, point );
	::SetCursor( ::LoadCursor(NULL, IDC_ARROW) );
	bFlTrack = FALSE;
 }

void TFullScrWindow::EvRButtonUp( uint modKeys, TPoint& point )
 {
	TWindow::EvRButtonUp( modKeys, point );
	//CloseWindow();
	::PostMessage( HWindow, WM_CLOSE, 0, 0 );
 }

void TFullScrWindow::EvMouseMove( uint modKeys, TPoint& point )
 {
	if( modKeys & MK_LBUTTON )
	 {
		if( pbmpDIB )
		 {

	 int iPoW = iStaOrgX,
		  iPoH = iStaOrgY;
			int iDx = iBmpWidth - iBmpViewWidth,
		  iDy = iBmpHigh - iBmpViewHigh;
																  //WM_MOUSEMOVE
	 iPoW -= (TFullScrWindow::iMouseX - point.x);
	 iPoH -= (TFullScrWindow::iMouseY - point.y);

			if( iDx > 0 || iDy > 0 )
			 {
				if( iPoW > 0 ) iPoW = 0;
				else if( iPoW < -iDx ) iPoW = -iDx;
				if( iPoH > 0 ) iPoH = 0;
		 else if( iPoH < -iDy ) iPoH = -iDy;

		 if( iPoW != iStaOrgX || iPoH != iStaOrgY )
		  {
			 iStaOrgX = (iDx> 0) ? iPoW:0;
			 iStaOrgY = (iDy > 0) ? iPoH:0;
			 ::SetViewportOrg( hdc, iStaOrgX, iStaOrgY );

			 ::InvalidateRect( HWindow, NULL, TRUE );
		  }
			 }
		 }
		//msg.Result = 0;
	 }
	else DefaultProcessing();

	TFullScrWindow::iMouseX = point.x;
	TFullScrWindow::iMouseY = point.y;
 }

void TFullScrWindow::EvPaletteChanged( HWND hWndPalChg )
 {
	if( hWndPalChg == HWindow )
	  TWindow::EvPaletteChanged( hWndPalChg );
	else EvQueryNewPalette();
 }

bool TFullScrWindow::EvQueryNewPalette()
 {
	HPALETTE HpalOld;
	int i;
	HPALETTE hp = hpalAlias ? hpalAlias:(HPALETTE)pal;
					  //TPalette
	if( !hp ) return 0;

	if( !hdc ) hdc = ::GetDC( HWindow );
	HpalOld = ::SelectPalette( hdc, hp, FALSE );

	i = ::RealizePalette( hdc );

	::SelectPalette( hdc, HpalOld, FALSE);

	if (i > 0)
	  ::InvalidateRect( HWindow, NULL, TRUE);

	return i;
 }



DEFINE_RESPONSE_TABLE1( TFullScrWindow, TWindow )
  EV_WM_ERASEBKGND,
  EV_WM_KEYDOWN,
  EV_WM_SYSKEYDOWN,
  EV_WM_MOUSEMOVE,
  EV_WM_LBUTTONDOWN,
  EV_WM_LBUTTONUP,
  EV_WM_RBUTTONUP,
  EV_WM_SETCURSOR,
  EV_WM_QUERYNEWPALETTE,
  EV_WM_PALETTECHANGED,
END_RESPONSE_TABLE;


