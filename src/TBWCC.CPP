#include <bwcc.h>
#include <ctl3d.h>

#include <owl\owlpch.h>
//#include <owl\dialog.h>
#include <owl\module.h>
#include <except.h>

#include "tbwcc.hpp"

static const char far* cErrTtl = "Error";

static void near pascal EraseModule( TModule*& BWCCModule )
 {
	if( BWCCModule )
	 {
		delete  BWCCModule;
		BWCCModule = 0;
	 }
 }

TBWCC::TBWCC()
 {
	BWCCModule = 0; bCtl3D = 0; hMod = 0;
	iStatus = 1;

	try
	 {
		BWCCModule = new TModule( "BWCC.DLL", true );
	 }
  catch( TXOwl& ex )
	 {
		bool bFlFound;

		::MessageBox( 0,
		  ex.ResourceIdToString( &bFlFound, ex.GetErrorCode(), ::Module ).c_str(),
		  cErrTtl,
		  MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL );
		iStatus = 0;
		EraseModule( BWCCModule );
		return;
	 }
	catch( xmsg& x )
	 {
		::MessageBox( 0, x.why().c_str(), cErrTtl,
						  MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL );
		iStatus = 0;
		EraseModule( BWCCModule );
		return;
	 }
	catch( ... )
	 {
		::MessageBox( 0, "Error CTL loading", cErrTtl,
						  MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL );
		iStatus = 0;
		EraseModule( BWCCModule );
		return;
	 }

	bool FAR PASCAL(*bwccIntlInit)(uint);
	(FARPROC)bwccIntlInit = BWCCModule->GetProcAddress("BWCCIntlInit");
	if (bwccIntlInit)
	  bwccIntlInit(0 );

	bool FAR PASCAL(*bwccRegister)(HINSTANCE);
	(FARPROC)bwccRegister = BWCCModule->GetProcAddress("BWCCRegister");
	if (bwccRegister)
	  bwccRegister( ::Module->GetInstance() );
	else
	 {
		EraseModule( BWCCModule );
		//delete BWCCModule; BWCCModule = 0;
	 }

	bCtl3D = Ctl3dRegister( (hMod=::Module->operator HINSTANCE()) );
	if( bCtl3D ) Ctl3dAutoSubclass( hMod );
 }

TBWCC::~TBWCC()
 {
	if (BWCCModule)
	 {
		bool FAR PASCAL(*bwccIntlTerm)(void);
		(FARPROC)bwccIntlTerm = BWCCModule->GetProcAddress("BWCCIntlTerm");
		if (bwccIntlTerm)
		  bwccIntlTerm();
		delete BWCCModule;
		BWCCModule = 0;
	 }

	if( bCtl3D ) { Ctl3dUnregister( hMod ); bCtl3D = 0; }
 }

