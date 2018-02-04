#define _MYDLL
#include "my_defs.hpp"

#include <typeinfo.h>
#include "centdlg.hpp"

void TCenterDialog::SetupWindow()
 {
	TDialog::SetupWindow();

	TRect r;
	GetWindowRect( r );
	RECT rd;
	::GetWindowRect( GetDesktopWindow(), &rd );
	r.left = (rd.right - r.right + r.left) / 2;
	r.bottom = (rd.bottom - r.bottom + r.top) / 2;
	::SetWindowPos( HWindow, 0, r.left, r.bottom, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
 }

TCenterDialog::~TCenterDialog()
 {
	liZOrder.Flush( 1 );
 }


void TCenterDialog::InitZOrder_()
 {
	TIDoubleListIteratorImp<TZPlacement> iter( liZOrder );
	TZPlacement *pPl = iter.Current();

	HWND hPrev = 0;
	for( ; int(iter); pPl = ++iter )
	 {
	  if( hPrev && typeid(*pPl) == typeid(TZCtl) )
		{
		  ::SetWindowPos( pPl->hwnd, hPrev, 0, 0, 0, 0,
								SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW
							 );
		  hPrev = pPl->hwnd;
		}
	  else if( typeid(*pPl) == typeid(TZGroup) )
		 hPrev = pPl->hwnd;

	}

	liZOrder.Flush( 1 );
 }

