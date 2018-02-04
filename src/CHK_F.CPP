#define _MYAPP
#include "my_defs.hpp"

#include <bwcc.h>

#include <owl\owlpch.h>
#include <owl\except.h>
#include <owl\gdibase.h>

#include "chk_app.hpp"
#include "chk_inst.hpp"


int OwlMain( int /*argc*/, char** /*argv[]*/ )
 {
	int iStatus = 0;

	if( !CheckInstall() )
	 {
		BWCCMessageBox( 0, "You must run CHKFLOP.COM before start Windows", "CHECKFLOP: Error",
							 MB_OK | MB_ICONSTOP | MB_TASKMODAL );
		return 1;
	 }

	try {
	  TChkFlopApp myApp( "ChkFlopApp" );
	  iStatus = myApp.Run();
	 }
	catch( TXInvalidMainWindow& x )
	 {
		BWCCMessageBox( 0, "Invalid", "CHECKFLOP: Error", MB_OK | MB_ICONSTOP | MB_TASKMODAL );
	 }
	catch(TGdiBase::TXGdi& x )
	 {
		BWCCMessageBox( 0, x.Msg(x.GetErrorCode(), ::Module->GetInstance()).c_str(),
						  "CHECKFLOP: Error", MB_OK | MB_ICONSTOP | MB_TASKMODAL );
		iStatus = -1;
	 }
	catch( xmsg& x )
	 {
		iStatus = HandleGlobalException( x,
		  "Abnormal Termination, uncaught xmsg", 0 );
	 }
	catch( ... )
	 {
		iStatus = HandleGlobalException( xmsg(string()),
		  "Abnormal Termination, uncaught ...", 0 );
	 }

	return iStatus;
 }

