#if !defined(_CHKFLOP_APP_)
#define _CHKFLOP_APP_

#include <owl\owlpch.h>
#include <owl\applicat.h>
#include <owl\framewin.h>
#include <owl\gdiobjec.h>

#include "chk_drv.hpp"
extern "C" {
  #include "appexec.h"
 }
#include "synth.hpp"
#include "e_ctrl.hpp"

#define iNumbMsg  22

class TDriverErrLoad {};
class TDriverErrRegister {};

class TMyFrame: public TFrameWindow
 {
public:
  TMyFrame( TWindow* parent, const char far *title = 0,
				TWindow* clientWnd = 0, bool shrinkToClient = false,
				TModule* module = 0
			 ); 

  virtual ~TMyFrame();
  void EvTimer( uint timerId );
  virtual bool CanClose();

protected:
  virtual char far* GetClassName();
  virtual void GetWindowClass( WNDCLASS& wndClass );
  virtual void SetupWindow();

  void EvShowWindow( bool show, uint status );
  void EvSysCommand( uint cmdType, TPoint& point );
  void EvSysColorChange();

  LRESULT EvUninstall( WPARAM wParam, LPARAM lParam );
  LRESULT EvStartAvir( WPARAM wParam, LPARAM lParam );

  virtual void Destroy( int retVal = 0 );
  void RefreshIcon( bool bFlRedraw = false );

  DECLARE_RESPONSE_TABLE( TMyFrame );

private:
  HDRVR hDrv;
  bool bFlReg;
  HTASK hTask;

  TIcon ic1, ic2, icAct1, icAct2;
  HICON hcOld;
  bool bFlGlobalEnbl;

  uint16 uiTimer;
  EXECAPPINFO execInf;

  int ExecuteApp( string& sPath, string& sParam );

  TSynthSelector sSel;
  TExecControl   eControl;

  bool bFlBlink;

  void About();
  void ErrExec( int iRes );
  static const char* cErrStr[ iNumbMsg ];
 };


class TChkFlopApp: public TApplication
 {
public:
  TChkFlopApp( const char far* name,  TModule*& gModule = ::Module, TAppDictionary*
			appDict = 0 ):
			TApplication( name, gModule, appDict ) {};

  void InitMainWindow();
  //virtual int Error( int errorCode );
  //virtual int Error( xmsg& x, unsigned captionResId, unsigned promptResId );
 };


#endif
