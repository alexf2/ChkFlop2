#if !defined(_FULLSCR_WINDOW_)
#define  _FULLSCR_WINDOW_

#include <owl\owlpch.h>
#include <owl\gdiobjec.h>
#include <owl\window.h>


/*#undef _EXPORT2
#if defined(_FULLSCRDLL) || defined(_ALLDLGDLL)
  #define _EXPORT2 _export
#else
  #define _EXPORT2 _CLASSTYPE
#endif
*/

#define WM_USER_FULLSCRDONE (WM_USER + 5)
#define _EXPORT2


_CLASSDEF(TFullScrWindow)
_CLASSDEF(TSysPaletteSave)

const int iNumbEnt = 21;

class _EXPORT2 TSysPaletteSave
 {
public:
  TSysPaletteSave() { hdc = NULL; }
  ~TSysPaletteSave();

  void operator()( HDC );
  void operator~();

  void Reset();

private:
  static int iIndexPal[ iNumbEnt ];
  static COLORREF rgb[ iNumbEnt ];
  HDC hdc;
  int iNumColors;
 };

class _EXPORT2 TFullScrWindow: public TWindow
 {
public:
  TFullScrWindow( TWindow* parent, TDib*  pbmpDIB_, HPALETTE hpalAlias_, TModule* AModule = 0 );

  virtual ~TFullScrWindow();

  virtual void Destroy( int retVal = 0 );

protected:
  virtual LPSTR GetClassName();      //TMDIFrame
  virtual void GetWindowClass( WNDCLASS & AWndClass );
  //virtual void SetupWindow();
  virtual BOOL Create();

  //void WMEraseBkGnd( RTMessage ) = [WM_FIRST + WM_ERASEBKGND];

  virtual void Paint( TDC& dc, bool erase, TRect& rect );

  //void WMKeyDown( RTMessage ) = [WM_FIRST + WM_KEYDOWN];
  //void WMSysKeyDown( RTMessage ) = [WM_FIRST + WM_SYSKEYDOWN];
  //void WMMouseMove( RTMessage msg ) = [WM_FIRST + WM_MOUSEMOVE];
  //void WMLButtonDown( RTMessage ) = [WM_FIRST + WM_LBUTTONDOWN];
  //void WMRButtonUp( RTMessage ) = [WM_FIRST + WM_RBUTTONUP];
  //void WMLButtonUp( RTMessage ) = [WM_FIRST + WM_LBUTTONUP];

  //void WMSetCursor( RTMessage ) = [WM_FIRST + WM_SETCURSOR];

  //void WMQueryNewPalette( RTMessage ) = [WM_FIRST + WM_QUERYNEWPALETTE];
  //void WMPaletteChanged( RTMessage ) = [WM_FIRST + WM_PALETTECHANGED];

  bool EvEraseBkgnd( HDC );
  void EvKeyDown( uint key, uint repeatCount, uint flags );
  void EvSysKeyDown( uint key, uint repeatCount, uint flags );
  void EvMouseMove( uint modKeys, TPoint& point );
  void EvLButtonDown( uint modKeys, TPoint& point );
  void EvLButtonUp( uint modKeys, TPoint& point );
  void EvRButtonUp( uint modKeys, TPoint& point );
  bool EvSetCursor( HWND hWndCursor, uint hitTest, uint mouseMsg );
  bool EvQueryNewPalette();
  void EvPaletteChanged( HWND hWndPalChg );

private:
  TPalette pal;
  TBitmap  *pBmp;

  int iBmpWidth, iBmpHigh;
  int iBmpViewWidth, iBmpViewHigh;
  TDib*  pbmpDIB;
  HPALETTE hpalAlias;
  HCURSOR hcHand;
  HDC hdc;
  BOOL bFlTrack;
  int iStaOrgX, iStaOrgY;
  RECT r;

  TSysPaletteSave spsSysPalette;

  static int iMouseX, iMouseY;

  BOOL bFlOldDisable;

  void PrepareKey( uint16 uiKey );

  void near pascal DrawBitmapDIBWithPalette( HDC Hdc, HPALETTE hpalAlias, int X, int Y );
  void near pascal DrawBitmapDIB( HDC Hdc, int X, int Y );

  DECLARE_RESPONSE_TABLE( TFullScrWindow );
 };

#endif

