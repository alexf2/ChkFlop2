#define _MYDLL
#include "my_defs.hpp"

#include "opt_dlg.hpp"
#include "lim_opt.hpp"
#include "RC\chk_cfg.rh"

#include <ctl3d.h>
#include <typeinfo.h>
#include <stdio.h>
#include <dir.h>


#include <CLASSLIB/dlistimp.h>

const int LIM_MaxSld = 10000;

const int iWidthSlider  = 90,
			 iHighSlider   = 220,
			 iWidthSlider2 = 50,
			 iHighSlider2  = 700;

static const int iZOrderId[] =
	 {
		IDC_EDIT_AV,
		IDC_EDIT_PREF,
		IDC_EDIT_DRIVE,
		IDC_EDIT_POST,
		IDC_EDIT_MEM,

		IDC_BTN_UPDATE,
		IDC_BTN_ADD,
		IDC_BTN_REMOVE,
		IDC_BTN_REMOVEALL,
		IDC_BTN_BROWSE,

		IDC_GRP_OPT,
		IDC_CHK_CHECKSERIAL,
		IDC_SLD_QUEUE,
		IDC_SLD_RETRYDOS,
		IDC_SLD_RETRYWIN,

		IDC_CHECK_DOSA,
		IDC_CHECK_DOSB,
		IDC_CHECK_WINA,
		IDC_CHECK_WINB,

		IDOK,
		IDCANCEL,
		0
	 };

const int iNumbHdrs = 5;
struct TGDescr
 {
	char *pHdr;
	int  iPercent;
	int  iTiledId;
 };
static const TGDescr cLstHdr[ iNumbHdrs ] =
 {
	{ "Anti-virus path", 40, IDC_EDIT_AV },
	{ "Prefix", 17, IDC_EDIT_PREF },
	{ "Mask", 13, IDC_EDIT_DRIVE },
	{ "Postfix", 17, IDC_EDIT_POST },
	{ "DOS", 13, IDC_EDIT_MEM }
 };


void TDRangeVal::Error()
 {
	string msgTmpl = ::Module->LoadString( IDS_VALNOTINRANGE ).c_str();

	char* msg = new char[ msgTmpl.length() + 10 + 10 + 1 ];
	wsprintf( msg, msgTmpl.c_str(), Min, Max );
	::MessageBox( 0, msg, "Error",
	  MB_ICONEXCLAMATION | MB_OK | MB_TASKMODAL );
	delete [] msg;
 }

bool TDRangeVal::IsValid( const char far* str )
 {
	if( !*str ) return true;
	return TRangeValidator::IsValid( str );
 }

static void near pascal SetupSld( TSlider* pSld, uint16 uiMin,
											 uint16 uiMax, int iStep, int iFix,
											 uint16 uiPos
										  )
 {
	pSld->SetRange( uiMin, uiMax );
	pSld->SetRuler( iStep, iFix );
	pSld->SetPosition( uiPos );
 }


TOptionsDialog::TOptionsDialog( TParamBlock_Load& rINI_,
										  const char far *title,
										  TWindow* parent,
										  TResId resId,
										  TModule* module
 ):
	rINI( rINI_ ),
	TCenterDialog( parent, resId, module ),
	TLayoutWindow( parent, title, module ),
	TWindow( parent, title, module ),
	pTtl( const_cast<char*>(title) ),
	bFlDirty( false )
	//rcBrush( new TBrush(TColor::LtGray) )
 {
	CreateBrush();

	rcListBox = new TAVListBox( rINI_, this, IDC_LIST, module );

	rcSlQueue     = new  THSlider( this, IDC_SLD_QUEUE, 0, 0, 0, 0,
											 IDB_HSLIDERTHUMB, module );
	rcSlRetryDOS  = new  THSlider( this, IDC_SLD_RETRYDOS, 0, 0, 0, 0,
											 IDB_HSLIDERTHUMB, module );
	rcSlRetryWin  = new  THSlider( this, IDC_SLD_RETRYWIN, 0, 0, 0, 0,
											 IDB_HSLIDERTHUMB, module );
	rcSlQueue()->Attr.Style |= WS_GROUP | WS_TABSTOP;
	rcSlRetryDOS()->Attr.Style |= WS_GROUP | WS_TABSTOP;
	rcSlRetryWin()->Attr.Style |= WS_GROUP | WS_TABSTOP;


	rcTxtQueue    = new TStatic( this, IDC_TXT_QUEUE, 0, module );
	rcTxtRetryDOS = new TStatic( this, IDC_TXT_RETRYDOS, 0, module );
	rcTxtRetryWin = new TStatic( this, IDC_TXT_RETRYWIN, 0, module );

	rcTxtQueueV    = new TStatic( this, IDC_TXT_QUEUEV, "**",
											0, 0, 0, 0, 5, module );
	rcTxtRetryDOSV = new TStatic( this, IDC_TXT_RETRYDOSV, "**",
											0, 0, 0, 0, 5, module );
	rcTxtRetryWinV = new TStatic( this, IDC_TXT_RETRYWINV, "**",
											0, 0, 0, 0, 5, module );

	rcEditAV   = new TEditAS( this, IDC_EDIT_AV, 256, module );
	rcEditPref = new TEditAS( this, IDC_EDIT_PREF, 256, module );
	rcEditMask = new TEditAS( this, IDC_EDIT_DRIVE, 256, module );
	rcEditPost = new TEditAS( this, IDC_EDIT_POST, 256, module );
	rcEditMem  = new TEditAS( this, IDC_EDIT_MEM, 10, module );
	rcEditMem()->SetValidator(
	  new TDRangeVal(LIM_DOSMem_MIN, LIM_DOSMem_MAX)
									 );

	rcListTtl = new TGadgetWindow( this, TGadgetWindow::Horizontal,
	  new TFont("MS Sans Serif", 8, 0, 0, 0, FW_BOLD), module );

	for( int i = 0; i < iNumbHdrs; i++ )
	 {
		 TTextGadget* pG;
		 rcListTtl()->Insert(
			*(pG = new TTextGadget( i + 1, TGadget::Raised, TTextGadget::Center,
									strlen(cLstHdr[i].pHdr), cLstHdr[i].pHdr
								 )) );
		 if( i < iNumbHdrs - 1  )
			rcListTtl()->Insert( *new TSeparatorGadget(2) );
	 }


	rcListBox()->AssFormat( rcListTtl() );
 }

TOptionsDialog::~TOptionsDialog()
 {
	//if( pListBox ) { delete pListBox; pListBox = 0; }
	//rcTxtQueue.ClearRc();
	//rcTxtRetryDOS.ClearRc();
	//rcTxtRetryWin.ClearRc();
 }

void near pascal TOptionsDialog::AddLay( TWindow *pWnd )
 {
	TLayoutMetrics m;
	m.SetMeasurementUnits( lmPixels );

	m.X.Absolute( lmLeft, pWnd->Attr.X );
	m.Y.Absolute( lmTop, pWnd->Attr.Y );
	m.Width.Absolute( pWnd->Attr.W );
	m.Height.Absolute( pWnd->Attr.H );
	SetChildLayoutMetrics( *pWnd, m );
 }

void near pascal TOptionsDialog::SetPlacement( TWindow *pCtl,
															  TWindow *pInd,
															  TWindow *pWndRect,
															  TWindow *pWndPos
															)
 {
	TLayoutMetrics m;
	m.SetMeasurementUnits( lmPixels );
	int iDst = pWndPos->Attr.H / 4;
	iDst = iDst ? iDst:1;

	m.X.Set( lmLeft, lmSameAs, pWndPos, lmLeft, 0 );
	m.Y.Below( pWndPos, iDst );
	m.Width.PercentOf( pWndRect, iWidthSlider, lmWidth );
	m.Height.PercentOf( pWndRect, iHighSlider, lmHeight );

	SetChildLayoutMetrics( *pCtl, m );

	m.X.RightOf( pCtl, iDst );
	m.Y.SameAs( pCtl, lmTop );
	m.Width.Absolute( iWidthLbl );
	m.Height.SameAs( pWndRect, lmHeight );

	SetChildLayoutMetrics( *pInd, m );
 }


static void near pascal ScToCl( TWindow *pWin, TRect *pRect )
 {
	pWin->ScreenToClient( *(TPoint*)pRect );
	pWin->ScreenToClient( *(((TPoint*)pRect)+1) );
 }


void near pascal TOptionsDialog::AcqureListViewer( TListBox* pLBox, TGadgetWindow* pSbar )
 {
	TLayoutMetrics m;
	m.SetMeasurementUnits( lmPixels );

	AddLay( pLBox );

	m.X.SameAs( pLBox, lmLeft );
	m.Y.Above( pLBox, 0 );
	m.Width.SameAs( pLBox, lmWidth );
	m.Height.Absolute( 1.6 * iHighLbl );

	SetChildLayoutMetrics( *pSbar, m );
	Layout();

	TGadget *pG = pSbar->FirstGadget();
	int w = pLBox->Attr.W;
	for( int i = 0; pG; pG = pSbar->NextGadget(*pG) )
	 {
		if( typeid(*pG) != typeid(TTextGadget) ) continue;

		TRect& rR = pG->GetBounds();
		TSize sz; sz.cy = rR.bottom - rR.top + 2;

		if( i != iNumbHdrs - 1 )
		 {
			pG->SetShrinkWrap( false, true );
			sz.cx = w * cLstHdr[i++].iPercent / 100;
		 }
		else
		 {
			pSbar->Remove( *pG );
			pSbar->LayoutSession();
			pG->WideAsPossible = true;
			pSbar->Insert( *pG );
			pSbar->LayoutSession();

			break;
		 }

		pG->SetSize( sz );
	 }

	TWindow *pW = ChildWithId( cLstHdr[0].iTiledId );
	TRect r = pW->GetWindowRect();
	ScToCl( this, &r );
	int iSta = r.left;
				//iSta = 0;
	pG = pSbar->FirstGadget();
	for( i = 0; pG; pG = pSbar->NextGadget(*pG) )
	 {
		if( typeid(*pG) != typeid(TTextGadget) ) continue;

		TRect& rR = pG->GetBounds();
		pW = ChildWithId( cLstHdr[i++].iTiledId );
		r = pW->GetWindowRect();
		ScToCl( this, &r );
		r.left = iSta + rR.left; r.right = iSta + rR.right;

		pW->SetWindowPos( 0, r, SWP_NOZORDER | SWP_NOREDRAW );
	 }
 }

void near pascal TOptionsDialog::GetFontInfo( TEXTMETRIC& tm )
 {
	HDC         hDC = ::GetDC( 0 );
	HFONT       hFont = (HFONT)HandleMessage( WM_GETFONT );
	HGDIOBJ     hOldFont;

	if( hFont ) hOldFont = ::SelectObject( hDC, hFont );
	::GetTextMetrics( hDC, &tm );
	if( hFont ) ::SelectObject( hDC, hOldFont );

	::ReleaseDC(0, hDC);
 }

void near pascal TOptionsDialog::UpdateLbls( TSlider* pSld )
 {
	HandleMessage( WM_HSCROLL, SB_THUMBPOSITION,
						MAKELPARAM(pSld->GetPosition(), pSld->HWindow) );
 }

void TOptionsDialog::SetupWindow()
 {
	TCenterDialog::SetupWindow();

	HFONT hFont = (HFONT)HandleMessage( WM_GETFONT );
	SendDlgItemMessage( IDC_TXT_QUEUEV, WM_SETFONT, (WPARAM)hFont, 0 );
	SendDlgItemMessage( IDC_TXT_RETRYDOSV, WM_SETFONT, (WPARAM)hFont, 0 );
	SendDlgItemMessage( IDC_TXT_RETRYWINV, WM_SETFONT, (WPARAM)hFont, 0 );

	SendDlgItemMessage( IDC_SLD_QUEUE, WM_SETFONT, (WPARAM)hFont, 0 );
	SendDlgItemMessage( IDC_SLD_RETRYDOS, WM_SETFONT, (WPARAM)hFont, 0 );
	SendDlgItemMessage( IDC_SLD_RETRYWIN, WM_SETFONT, (WPARAM)hFont, 0 );

	TEXTMETRIC tm;
	GetFontInfo( tm );
	iWidthLbl = tm.tmAveCharWidth * 2.5;
	iHighLbl = tm.tmHeight;

	SetCaption( pTtl );
	Ctl3dSubclassDlgEx( HWindow, CTL3D_ALL );

	AddLay( rcTxtQueue() );
	AddLay( rcTxtRetryDOS() );
	AddLay( rcTxtRetryWin() );

	SetPlacement( rcSlQueue(), rcTxtQueueV(), rcTxtQueue(), rcTxtQueue() );
	SetPlacement( rcSlRetryDOS(), rcTxtRetryDOSV(), rcTxtQueue(), rcTxtRetryDOS() );
	SetPlacement( rcSlRetryWin(), rcTxtRetryWinV(), rcTxtQueue(), rcTxtRetryWin() );

	AcqureListViewer( rcListBox(), rcListTtl() );

	Layout();

	SetupSld( rcSlQueue(), LIM_Queue_MIN, LIM_Queue_MAX, 1, 0,
				 rINI.opt.iSizeOfSerialQueue
			  );
	SetupSld( rcSlRetryDOS(), LIM_Retry_MIN, LIM_Retry_MAX, 1, 0,
				 rINI.opt.iRetryNumberSec1ReadDOS
			  );
	SetupSld( rcSlRetryWin(), LIM_Retry_MIN, LIM_Retry_MAX, 1, 0,
				 rINI.opt.iRetryNumberSec1ReadWin
			  );

	SendDlgItemMessage( IDC_CHK_CHECKSERIAL, BM_SETCHECK,
							  rINI.opt.flCheckSerial, 0 );

	UpdateLbls( rcSlQueue() );
	UpdateLbls( rcSlRetryDOS() );
	UpdateLbls( rcSlRetryWin() );

	*this + new TZGroup( GetDlgItem(IDC_LIST) );
	for( int i = 0; iZOrderId[i]; i++ )
	  *this + new TZCtl( GetDlgItem(iZOrderId[i]) );
	InitZOrder_();

	SelectAndRefresh( 0 );
	TestAndSwitch( 'A', IDC_CHECK_DOSA );
	TestAndSwitch( 'B', IDC_CHECK_DOSB );
	TestAndSwitch( 'A', IDC_CHECK_WINA );
	TestAndSwitch( 'B', IDC_CHECK_WINB );
 }

void near pascal TOptionsDialog::SelectAndRefresh( int iInd )
 {
	rcListBox()->SetSelIndex( iInd );
	PostMessage( WM_COMMAND, rcListBox()->GetId(),
					 MAKELONG(rcListBox()->HWindow, LBN_SELCHANGE)
				  );
 }

void near pascal TOptionsDialog::TestAndSwitch( int iDrive, int id )
 {
	bool bFl = (GetDriveType( iDrive - 'A' ) != 0);
	if( !bFl ) SendDlgItemMessage( id, BM_SETCHECK, 1, 0 );
   ::EnableWindow( GetDlgItem(id), bFl );
 }

void TAVListBox::SetupWindow()
 {
	const double dWM = 0.002;
	TListBox::SetupWindow();

	TRect r;
	GetClientRect( r );
	iItemW = r.right - r.left;
	if( !(TMemoryDC*)dbBuf )
	  {
		 TClientDC dcDC( HWindow );
		 dbBuf.Init( dcDC, iItemW, iItemH );
	  }

	iWMarg = double(iItemW) * dWM; iWMarg = iWMarg ? iWMarg:1;

	TISDoubleListIteratorImp<TAVir> iter( rINI.liAvir );
	 TAVir *pAV = iter.Current();
	 for( int i = 0; int(iter); pAV = ++iter, i++ )//TListBox
	  {
		 char cBuf[10];
		 sprintf( cBuf, "%u", i );
		 InsertString( (char*)int(*pAV), -1 );
	  }  //TListBox
 }


char far* TOptionsDialog::GetClassName()
 {
	return "CHKFLOP_CFGWIN";
 }

void TOptionsDialog::GetWindowClass( WNDCLASS& wndClass )
 {
	TCenterDialog::GetWindowClass( wndClass );
 }

void TOptionsDialog::CmOk()
 {
	rINI.opt.iSizeOfSerialQueue = rcSlQueue()->GetPosition();
	rINI.opt.flCheckSerial =
	  SendDlgItemMessage( IDC_CHK_CHECKSERIAL, BM_GETCHECK, 0, 0 );
	rINI.opt.iRetryNumberSec1ReadDOS = rcSlRetryDOS()->GetPosition();
	rINI.opt.iRetryNumberSec1ReadWin = rcSlRetryWin()->GetPosition();

	TCenterDialog::CmOk();
 }

void TOptionsDialog::CmRmAll()
 {
	rINI.liDOSMem.Flush( 1 );
	rINI.liDisksDOS.Flush( 1 );
	rINI.liDisksWin.Flush( 1 );
	rINI.liAvir.Flush( 1 );

	rcListBox()->ClearList();

	SelectAndRefresh( 0 );
	bFlDirty = true;
 }

void near pascal TOptionsDialog::DetachAV( int iIndDta, TIDblListImpDel<TAVDescr>& liDisks )
 {
	TISDoubleListIteratorImp<TAVDescr> iter( liDisks );
	TAVDescr *pD = iter.Current();
	for( ; int(iter); pD = ++iter )
	  //pD->GetChain().Detach( iIndDta );
	  (*pD) -= iIndDta;
 }

void TOptionsDialog::CmRm()
 {
	int iInd = rcListBox()->GetSelIndex();
	if( iInd < 0 ) return;
	int iIndDta = rcListBox()->GetItemData( iInd );

	TDescriptor descr( iIndDta );

	TAVir *pAV = rINI.liAvir.Find2( (const TAVir*)((void*)&descr) );
	rINI.liAvir.Detach( pAV, 1 );

	TMemDescr *pMem = rINI.liDOSMem.Find2( (const TMemDescr*)((void*)&descr) );
	rINI.liDOSMem.Detach( pMem, 1 );

	DetachAV( iIndDta, rINI.liDisksDOS );
	DetachAV( iIndDta, rINI.liDisksWin );

	RemoveNullDrives( rINI.liDisksDOS );
	RemoveNullDrives( rINI.liDisksWin );

	rcListBox()->DeleteString( iInd );

	SelectAndRefresh( iInd ? (iInd-1):0  );
	bFlDirty = true;
 }

bool near pascal TOptionsDialog::UpdateInternal( int iIndDta )
 {
	char cBuf[300]; *cBuf = 0;
	char cBuf2[300];

	rcEditAV()->GetText( cBuf, 128 );
	*(int*)(cBuf + strlen(cBuf)) = 0x0020;
	rcEditPref()->GetText( cBuf+strlen(cBuf), 80 );
	rcEditMask()->GetText( cBuf2, 79 );
	strcat( cBuf, " #:" ); strcat( cBuf, cBuf2 );
	*(int*)(cBuf + strlen(cBuf)) = 0x0020;
	rcEditPost()->GetText( cBuf+strlen(cBuf), 80 );

	rcEditMem()->GetText( cBuf2, 10 );
	uint16 uiMem = 0;
	sscanf( cBuf2, "%u", &uiMem );
	uiMem = (uiMem >= LIM_DOSMem_MIN) ? uiMem:LIM_DOSMem_MIN;

	TDescriptor descr( iIndDta );

	TAVir *pAV = rINI.liAvir.Find2( (const TAVir*)((void*)&descr) );
	TAVir avTmp;
	try {                     //////////
	  avTmp = cBuf;
	 }
	catch( TAErr& aeErr )
	 {
		char cTmp[ 257 ];
		int iLen = abs(aeErr.p1-aeErr.p2) + 1;
		iLen = iLen < 256 ? iLen:256;
		strncpy( cTmp, aeErr.p1, iLen );
		cTmp[ iLen ] = 0;

		sprintf( cBuf2, "Invalid format: %s\n", cTmp );
		::MessageBox( HWindow, cBuf2, "Error input", MB_OK | MB_ICONEXCLAMATION );
		return false;
	 }

	*pAV = cBuf;
	TMemDescr *pMem = rINI.liDOSMem.Find2( (const TMemDescr*)((void*)&descr) );
	pMem->uiMem = uiMem;

	ClearChain( iIndDta );
	//if( SendDlgItemMessage(IDC_CHECK_DOSA, BM_GETCHECK, 0, 0) )
	  AddChain( rINI.liDisksDOS, 'A', iIndDta, IDC_CHECK_DOSA );
	//if( SendDlgItemMessage(IDC_CHECK_DOSB, BM_GETCHECK, 0, 0) )
	  AddChain( rINI.liDisksDOS, 'B', iIndDta, IDC_CHECK_DOSB );
	//if( SendDlgItemMessage(IDC_CHECK_WINA, BM_GETCHECK, 0, 0) )
	  AddChain( rINI.liDisksWin, 'A', iIndDta, IDC_CHECK_WINA );
	//if( SendDlgItemMessage(IDC_CHECK_WINB, BM_GETCHECK, 0, 0) )
	  AddChain( rINI.liDisksWin, 'B', iIndDta, IDC_CHECK_WINB );

	RemoveNullDrives( rINI.liDisksDOS );
	RemoveNullDrives( rINI.liDisksWin );

	return true;
 }

void TOptionsDialog::CmUpdate()
 {
	int iInd = rcListBox()->GetSelIndex();
	if( iInd < 0 ) return;

	UpdateInternal( rcListBox()->GetItemData(iInd) );

	TRect r;  rcListBox()->GetItemRect( iInd, r );
	rcListBox()->InvalidateRect( r, false );
	bFlDirty = true;
 }

void TOptionsDialog::CmAdd()
 {
	int iInd = rINI.GetFreeIndex();
	TAutoMemoryRc<TAVir*> rcA( new TAVir() );
	rcA()->ChgID( iInd );
	TAutoMemoryRc<TMemDescr*> rcM( new TMemDescr() );
	rcM()->ChgID( iInd );

	TAVir* pA = rcA();  rcA.ClearRc();
	TMemDescr* pM = rcM(); rcM.ClearRc();
	rINI.liAvir.Add( pA );
	rINI.liDOSMem.Add( pM );

	if( UpdateInternal(iInd) )
	 {
		rcListBox()->InsertString( (char*)iInd, -1 );
		rcListBox()->SetSelIndex( rcListBox()->GetCount() - 1 );
	 }
	else
	 {
		rINI.liAvir.Detach( pA, 1 );
		rINI.liDOSMem.Detach( pM, 1 );
	 }

	bFlDirty = true;
 }

void near pascal TOptionsDialog::ClearChain( int iInd )
 {
	TISDoubleListIteratorImp<TAVDescr> iter( rINI.liDisksWin );
	TAVDescr *pD = iter.Current();
	for( ; int(iter); pD = ++iter ) (*pD) -= iInd;

	TISDoubleListIteratorImp<TAVDescr> iter2( rINI.liDisksDOS );
	pD = iter2.Current();
	for( ; int(iter2); pD = ++iter2 ) (*pD) -= iInd;
 }
void near pascal TOptionsDialog::AddChain( TIDblListImpDel<TAVDescr>& rLi,
  int iDsk, int iInd, int iIdCtl )
 {
	if( !::IsWindowEnabled(GetDlgItem(iIdCtl)) ||
		 !SendDlgItemMessage(iIdCtl, BM_GETCHECK, 0, 0)
	  ) return;

	TISDoubleListIteratorImp<TAVDescr> iter( rLi );
	TAVDescr *pD = iter.Current();
	bool bFlFound = false;
	for( ; int(iter); pD = ++iter )
	 if( int(*pD) == iDsk )
	  {
		 (*pD) += iInd;
		 bFlFound = true;
	  }


	if( bFlFound == false )
	 {
		rLi.Add( (pD = new TAVDescr(iDsk)) );
		(*pD) += iInd;
	 }
 }

void near pascal TOptionsDialog::RemoveNullDrives( TIDblListImpDel<TAVDescr>& rLi )
 {
	TISDoubleListIteratorImp<TAVDescr> iter( rLi );
	TAVDescr *pD = iter.Current();
	for( ; int(iter); pD = ++iter )
	  if( !pD->GetChain().GetItemsInContainer() )
		{
		  rLi.Detach( pD, 1 );
		  RemoveNullDrives( rLi );
		  return;
		}
 }

void TOptionsDialog::CmBrowse()
 {
	char cBuf[257];
	getcwd( cBuf, 256 );

	TOpenSaveDialog::TData  fData(
	  OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST,
	  "Executable Files (*.exe;*.com;*.pif)|*.exe;*.com;*.pif|All Files (*.*)|*.*|",
	  0, cBuf, "*" );

	TFileOpenDialog fdBrowse( this, fData, 0, "Browse anti-virus", ::Module );
	if( fdBrowse.Execute() != IDOK ) return;

	rcEditAV()->SetText( fData.FileName );

	rcEditPref()->SetText( "" );
	rcEditMask()->SetText( "\\*.*" );
	rcEditPost()->SetText( "/s" );

	sprintf( cBuf, "%u", 70 );
	rcEditMem()->SetText( cBuf );

	CheckDOS( 'A', true ); CheckDOS( 'B', true );
	CheckWin( 'A', true ); CheckWin( 'B', true );

	rcEditPost()->SetFocus();
 }

TAVListBox::TAVListBox( TParamBlock_Load& rINI_, TWindow* parent,
								int resourceId, TModule* module ):
  TListBox( parent, resourceId, module ),
  rINI( rINI_ )
 {
	CreateBrush();
	pFormat = 0;
	rcFont = new TFont( "MS Sans Serif", 8, 0, 0, 0, FW_BOLD );
	SetBkgndColor( TColor::LtGray );
 }

HBRUSH TOptionsDialog::EvCtlColor(HDC hDC, HWND hWndChild, uint ctlType)
 {
	if( ctlType == CTLCOLOR_LISTBOX || ctlType == COLOR_SCROLLBAR ||
		 ctlType == CTLCOLOR_STATIC
	  )
	 {
		::SetBkColor( hDC, TColor::LtGray );
		return *rcBrush();
	 }

	return TCenterDialog::EvCtlColor( hDC, hWndChild, ctlType );
 }

void TOptionsDialog::CreateBrush()
 {
	rcBrush = new TBrush( TColor::LtGray );
 }

void  TOptionsDialog::EvSysColorChange()
 {
	CreateBrush();
	Ctl3dColorChange();
	rcListBox()->EvSysColorChange_();
 }

void  TOptionsDialog::UpdateQueue( UINT uiCode )
 {
	UpdateSld( rcSlQueue(), rcTxtQueueV(), uiCode );
 }
void  TOptionsDialog::UpdateRDOS( UINT uiCode )
 {
	UpdateSld( rcSlRetryDOS(), rcTxtRetryDOSV(), uiCode );
 }
void  TOptionsDialog::UpdateRWin( UINT uiCode )
 {
	UpdateSld( rcSlRetryWin(), rcTxtRetryWinV(), uiCode );
 }

void  TOptionsDialog::UpdateSld( TSlider* pSld, TStatic* pTxt, uint16 uiCode )
 {
	char cTmp[25];

	sprintf( cTmp, "%u", pSld->GetPosition() );
	pTxt->SetText( cTmp );
 }

void  near pascal TOptionsDialog::CheckDOS( int iDrive, bool bFlCheck )
 {
	int id = (iDrive == 'A') ? IDC_CHECK_DOSA:IDC_CHECK_DOSB;
	if( !::IsWindowEnabled(GetDlgItem(id)) ) return;

	SendDlgItemMessage( id, BM_SETCHECK, bFlCheck, 0 );
 }

void  near pascal TOptionsDialog::CheckWin( int iDrive, bool bFlCheck )
 {
	int id = (iDrive == 'A') ? IDC_CHECK_WINA:IDC_CHECK_WINB;
	if( !::IsWindowEnabled(GetDlgItem(id)) ) return;

	SendDlgItemMessage( id, BM_SETCHECK, bFlCheck, 0 );
 }


void near pascal TOptionsDialog::LoadField( int iInd )
 {
	if( !rcListBox()->GetCount() )
	 {
		rcEditAV()->Clear();
		rcEditPref()->Clear();
		rcEditMask()->Clear();
		rcEditPost()->Clear();
		rcEditMem()->Clear();

		return;
	 }

	TAVir* pAV = 0;
	TMemDescr* pMem = 0;

	rcListBox()->AVirusWithId( iInd, pAV, pMem );

	/*rcEditAV()->Clear();
	rcEditPref()->Clear();
	rcEditMask()->Clear();
	rcEditPost()->Clear();
	rcEditMem()->Clear();*/

	rcEditAV()->SetText( pAV->Path() );
	rcEditPref()->SetText( pAV->Prefix() );
	rcEditMask()->SetText( pAV->Mask() );
	rcEditPost()->SetText( pAV->Postfix() );
							 //TControl
	char cBuf[25];
	sprintf( cBuf, "%u", pMem->uiMem );
	rcEditMem()->SetText( cBuf );

	CheckDOS( 'A', false ); CheckDOS( 'B', false );
	CheckWin( 'A', false ); CheckWin( 'B', false );

	TISDoubleListIteratorImp<TAVDescr> iter( rINI.liDisksDOS );
	TAVDescr *pD = iter.Current();
	for( ; int(iter); pD = ++iter )
	  CheckDOS( int(*pD), pD->GetChain().Find2((int)*pAV) ? true:false );

	TISDoubleListIteratorImp<TAVDescr> iter2( rINI.liDisksWin );
	pD = iter2.Current();
	for( ; int(iter2); pD = ++iter2 )
	  CheckWin( int(*pD), pD->GetChain().Find2((int)*pAV) ? true:false );
 }

void TOptionsDialog::Ev_CBN_Select()
 {
	LoadField( rcListBox()->GetItemData(rcListBox()->GetSelIndex()) );
	//TListBox
 }

void TAVListBox::AVirusWithId( int iInd, TAVir*& pAV, TMemDescr*& pMem )
 {
	TDescriptor descr( iInd );

	pAV  = rINI.liAvir.Find2( (const TAVir*)((void*)&descr) );
	pMem = rINI.liDOSMem.Find2( (const TMemDescr*)((void*)&descr) );
 }

static void near pascal CreateShortName( LPSTR LpFullName, int iLen,
													  char *pOut )
 {
	char Path[MAXPATH];
	int Len;


	//if( LpFileNameShort ) delete []LpFileNameShort; LpFileNameShort = NULL;

	_fstrcpy( Path, LpFullName );
	if( (Len = _fstrlen(LpFullName)) > iLen )
	 {
		LPSTR LpName;
		int   LenName;
		const char *StrIns = "\\...\\";

		for( LpName = LpFullName + Len - 1, LenName = 0;
			  LenName < Len-1 && *(LpName-1) != '\\';
			  LenName++, LpName-- );

		int SizeNeed = iLen - LenName - _fstrlen(StrIns);
		LPSTR LpPrev, LpCur;

		for( LpPrev = LpCur = Path, LenName = 0;
			  LenName <= SizeNeed && *LpCur;
			  LenName++, LpCur++ )
		  if( *LpCur == '\\' ) LpPrev = LpCur;
		*LpPrev = 0;
		_fstrcat( Path, StrIns ); _fstrcat( Path, LpName );
	 }

	//if( (LpFileNameShort = new char[ _fstrlen(Path) + 1 ]) )
	  //_fstrcpy( LpFileNameShort, Path );
	//else { Status = EM_OUTOFMEMORY; return; }
	strcpy( pOut, Path );
 }

static void near pascal CompressPath( char *cBuf, TMemoryDC& rDCMem, 
												  TRect& rT
												)
 {
	int iLen;
	TSize szExt( rDCMem.GetTextExtent(cBuf, (iLen=strlen(cBuf))) );
	if( szExt.cx <= rT.Width() ) return;

	float fMult = float(rT.Width()) / float(szExt.cx);
	char cTmp[270];

	while( szExt.cx > rT.Width() )
	 {
		*cTmp = 0;
		iLen = float(iLen) * fMult;
		CreateShortName( cBuf, iLen, cTmp );
		szExt = rDCMem.GetTextExtent( cTmp, (iLen=strlen(cTmp)) );
      fMult *= 0.9;
	 }

	strcpy( cBuf, cTmp );
 }

void TAVListBox::DrawItem( DRAWITEMSTRUCT far& dis )
 {
	//TListBox::DrawItem( dis );
	if( int(dis.itemID) < 0 || !(TMemoryDC*)dbBuf ) return;
	TAVir *pAV;
	TMemDescr *pMem;
	AVirusWithId( dis.itemData, pAV, pMem );
	if( !pAV ) return;

	TBrush *pBrushBkgnd, *pBrushRect;
	TColor iColorTxt;

	if( dis.itemState & ODS_SELECTED )
	 {
		pBrushBkgnd = rcBrushHigh();
		if( dis.itemState & ODS_FOCUS )
		 {
			pBrushRect = rcBrushRect();
			iColorTxt = TColor::LtYellow;
		 }
		else
		 {
			pBrushRect = rcBrushHigh();
			iColorTxt = TColor::White;
		 }
	 }
	else
	 {
		pBrushBkgnd = rcBrushNormal();
		pBrushRect = rcBrushNormal();
		iColorTxt = TColor::Black;
	 }


	TDC dcDC( dis.hDC );
	TMemoryDC& rDCMem = *dbBuf.BeginDrawing();

	//rDCMem.SelectObject( *pBrushBkgnd );
	TPoint pt;
	dbBuf.GetRect( pt );
	TRect rFill = dis.rcItem;
	rFill.top = 0; rFill.bottom = pt.y;
	rFill.right++; rFill.bottom++;
	rDCMem.FillRect( rFill, *pBrushBkgnd );

	rFill.right--; rFill.bottom--; rFill.Inflate( -1, -1 );
	rDCMem.FrameRect( rFill, *pBrushRect );
	//TDC TMemoryDC
	rDCMem.SetTextColor( iColorTxt );
	rDCMem.SetBkMode( TRANSPARENT );
	rDCMem.SetTextAlign( TA_LEFT | TA_TOP );
	//rDCMem.SetTextJustification( );
	//int breakExtra, int breakCount);

	rDCMem.SelectObject( *rcFont() );

	TGadget *pG = pFormat->FirstGadget();
	for( int i = 0; pG; pG = pFormat->NextGadget(*pG) )
	 {
		if( typeid(*pG) != typeid(TTextGadget) ) continue;

		TRect& rR = pG->GetBounds();

		TRect rT = rR;
		rT.top = 1; rT.bottom = pt.y - 2;
		rT.left += iWMarg; rT.right -= iWMarg;
		TPoint pSta( rT.left + iWMarg, iHMarg );

		char cBuf[270];
		switch( i++ )
		 {
			case 0:
			  sprintf( cBuf, "%s", pAV->Path() );
			  CompressPath( cBuf, rDCMem, rT );
			  break;

			case 1:
			  sprintf( cBuf, "%s", pAV->Prefix() );
			  break;
			case 2:
			  sprintf( cBuf, "#:%s", pAV->Mask() );
			  break;
			case 3:
			  sprintf( cBuf, "%s", pAV->Postfix() );
			  break;
			case 4:
			  sprintf( cBuf, "%u", pMem->uiMem );
			  break;
		 }


		rDCMem.ExtTextOut( pSta, ETO_CLIPPED, &rT, cBuf, strlen(cBuf) );
	 }

	TRect rcTmp( dis.rcItem );
	dbBuf.OutBufer( dcDC, rcTmp );
	dbBuf.EndDrawing();

	rDCMem.RestoreObjects();
 }

void TAVListBox::MeasureItem( MEASUREITEMSTRUCT far& mis )
 {
	const double dItemHeightMult = 1.5;

	//TListBox::MeasureItem( mis );
	HDC   hDC = ::GetDC( 0 );
	HGDIOBJ  hOldFont = ::SelectObject( hDC, (HFONT)*rcFont() );

	TEXTMETRIC  tm;
	::GetTextMetrics( hDC, &tm );

	::SelectObject( hDC, hOldFont );
	::ReleaseDC( 0, hDC );

	iItemH = mis.itemHeight = double(tm.tmHeight) * dItemHeightMult;
	iItemW = mis.itemWidth  = 640;
	iHMarg = double(iItemH - tm.tmHeight) / 2.0;
 }

/*bool TAVListBox::EvEraseBkgnd( HDC hdc )
 {

 }*/

void TAVListBox::CreateBrush()
 {
	rcBrushNormal = new TBrush( TColor::LtGray );
	rcBrushHigh = new TBrush( TColor::LtBlue );
	rcBrushRect = new TBrush( TColor::LtYellow );
 }

void TAVListBox::EvSysColorChange_()
 {
	CreateBrush();
 }

bool near pascal TAVListBox::IsEmpty()
 {
	//if( !GetCount() ) return true;
	//return GetCount()==1 && GetItemData(0)==-1 <= 1
   return false;
 }

bool TAVListBox::EvEraseBkgnd( HDC hdc )
 {
	/*if( !GetCount() )
	 {
		TClientDC dc( HWindow );
		//TDC dc( hdc );
		TRect r; GetClientRect( r );
		//RECT r; ::GetClientRect( HWindow, &r );

		//TRegion rgn;
		//dc.SelectClipRgn( rgn );

		rcBrushNormal()->UnrealizeObject();
		dc.FillRect( r, *(rcBrushNormal()) );
		//::FillRect( (HDC)dc, &r, (HBRUSH)::GetStockObject(GRAY_BRUSH) );

		return true;
	 }*/
	return TListBox::EvEraseBkgnd( hdc );
 }

TMemoryDC* TDrawBufer::BeginDrawing()
 {
	if( bFlCritical == false )
	 {
		rcCompatibleMemDC()->SelectObject( *rcCompatibleBmp() );
		bFlCritical = true;
	 }

	return rcCompatibleMemDC();
 }

void TDrawBufer::OutBufer( TDC& rDC, TRect& rR )
 {
	rcCompatibleMemDC()->SetMapMode( rDC.GetMapMode() );

	TPoint pt;
	rDC.GetViewportOrg( pt );
	rcCompatibleMemDC()->SetViewportOrg( pt );
	rDC.GetWindowOrg( pt );
	rcCompatibleMemDC()->SetWindowOrg( pt );

	BeginDrawing();
	TPoint pNul; pNul.x = pNul.y = 0;
	rDC.BitBlt( rR, *rcCompatibleMemDC(), pNul );
 }

void TDrawBufer::Restore()
 {
	if( bFlCritical == true )
	 {
		bFlCritical = false;
		rcCompatibleMemDC()->RestoreBitmap();
	 }
 }

TDrawBufer::~TDrawBufer()
 {
	Restore();
 }

void TDrawBufer::Init( TDC& rDC, int w, int h )
 {
	Restore();
	rcCompatibleMemDC = new TMemoryDC( rDC );
	rcCompatibleBmp = new TBitmap( rDC, (iW=w), (iH=h) );
 }

void TEditAS::EvSetFocus( HWND hWndLostFocus )
 {
	TEdit::EvSetFocus( hWndLostFocus );
	//SetSelection( 0,  );
	PostMessage( EM_SETSEL, 0, MAKELPARAM(0, -1) );
 }


DEFINE_RESPONSE_TABLE2( TOptionsDialog, TCenterDialog, TLayoutWindow )
  EV_CHILD_NOTIFY( IDOK, BN_CLICKED, CmOk ),

  EV_CHILD_NOTIFY( IDC_BTN_REMOVEALL, BN_CLICKED, CmRmAll ),
  EV_CHILD_NOTIFY( IDC_BTN_REMOVE, BN_CLICKED, CmRm ),
  EV_CHILD_NOTIFY( IDC_BTN_UPDATE, BN_CLICKED, CmUpdate ),
  EV_CHILD_NOTIFY( IDC_BTN_ADD, BN_CLICKED, CmAdd ),
  EV_CHILD_NOTIFY( IDC_BTN_BROWSE, BN_CLICKED, CmBrowse ),

  EV_WM_CTLCOLOR,
  EV_WM_SYSCOLORCHANGE,
  EV_CHILD_NOTIFY_ALL_CODES( IDC_SLD_QUEUE, UpdateQueue ),
  EV_CHILD_NOTIFY_ALL_CODES( IDC_SLD_RETRYDOS, UpdateRDOS ),
  EV_CHILD_NOTIFY_ALL_CODES( IDC_SLD_RETRYWIN, UpdateRWin ),
  EV_LBN_SELCHANGE( IDC_LIST, Ev_CBN_Select ),
END_RESPONSE_TABLE;

DEFINE_RESPONSE_TABLE1( TAVListBox, TListBox )
  EV_WM_ERASEBKGND,	
END_RESPONSE_TABLE;

DEFINE_RESPONSE_TABLE1( TEditAS, TEdit )
  EV_WM_SETFOCUS,
END_RESPONSE_TABLE;



