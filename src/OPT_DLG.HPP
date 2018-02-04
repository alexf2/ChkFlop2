#if !defined(_OPT_DLG_)
#define _OPT_DLG_

#include <owl\owlpch.h>
//#include <owl\applicat.h>
#include <owl\dialog.h>
#include <owl\layoutwi.h>
#include <owl\listbox.h>
#include <owl\slider.h>
#include <owl\dc.h>
#include <OWL\edit.h>

#include "centdlg.hpp"
#include "inidta.hpp"
#include "autorc.hpp"

class TEditAS: public TEdit
 {
public:
  TEditAS( TWindow* parent, int resourceID, uint textLen = 0,
			  TModule* module = 0 ):
	 TEdit( parent, resourceID, textLen, module ) {};


protected:
  void EvSetFocus( HWND hWndLostFocus );

private:
  DECLARE_RESPONSE_TABLE( TEditAS );
 };


class __rtti TDRangeVal: public TRangeValidator
 {
public:
  TDRangeVal( long min_, long max_ ):
	 TRangeValidator( min_, max_ ) {};

  virtual void Error();
  virtual bool IsValid( const char far* str );

 };

class TDrawBufer
 {
public:
  TDrawBufer() { iW = iH = 0; bFlCritical = false; }
  ~TDrawBufer();

  operator TMemoryDC*() { return rcCompatibleMemDC(); }

  void Init( TDC&, int w, int h );
  void OutBufer( TDC&, TRect& );

  TMemoryDC* BeginDrawing();
  void EndDrawing()
	{
	  Restore();
	}

  void GetRect( TPoint& p ) const { p.x = iW; p.y = iH; }

private:
  TAutoMemoryRc<TMemoryDC*> rcCompatibleMemDC;
  TAutoMemoryRc<TBitmap*> rcCompatibleBmp;
  int iW, iH;

  void Restore();
  bool bFlCritical;
 };

class TAVListBox: public TListBox
 {
public:
  TAVListBox( TParamBlock_Load& rINI_, TWindow* parent,
				  int resourceId, TModule* module = 0 );

  virtual void DrawItem( DRAWITEMSTRUCT far& drawInfo );
  virtual void MeasureItem( MEASUREITEMSTRUCT far& measureInfo );

  void AssFormat( TGadgetWindow *pF ) { pFormat = pF; }

  void AVirusWithId( int iInd, TAVir*& pAV, TMemDescr*& pMem );

  void  EvSysColorChange_();

protected:
  virtual void SetupWindow();
  bool EvEraseBkgnd( HDC );

private:
  TParamBlock_Load&  rINI;
  TGadgetWindow *pFormat;
  TAutoMemoryRc<TFont*> rcFont;
  TDrawBufer  dbBuf;

  bool near pascal IsEmpty();

  int iItemW, iItemH;
  int iHMarg, iWMarg;

  TAutoMemoryRc<TBrush*> rcBrushNormal, rcBrushHigh, rcBrushRect;
  void CreateBrush();

  DECLARE_RESPONSE_TABLE( TAVListBox );
 };

class TOptionsDialog: public TCenterDialog, public TLayoutWindow,
							 virtual public TWindow
 {
public:
	TOptionsDialog( TParamBlock_Load& rINI_,
						 const char far *title,
						 TWindow* parent,
						 TResId resId,
						 TModule* module = 0
					  );

	virtual ~TOptionsDialog();

   bool IsDirty() const { return bFlDirty; }
	//virtual bool  Find( TEventInfo& info, TEqualOperator op = 0 );

protected:
  char far* GetClassName();
  void GetWindowClass( WNDCLASS& wndClass );
  virtual void SetupWindow();

  void CmOk();

  void CmRmAll();
  void CmRm();
  void CmUpdate();
  void CmAdd();
  void CmBrowse();
										 //TDialog
private:
  TParamBlock_Load&  rINI;
  char *pTtl;
  int iWidthLbl, iHighLbl;
  bool bFlDirty;

  TAutoMemoryRc<TAVListBox*> rcListBox;
  TAutoMemoryRc<TSlider*> rcSlQueue, rcSlRetryDOS, rcSlRetryWin;

  TAutoMemoryRc<TStatic*> rcTxtQueue, rcTxtRetryDOS, rcTxtRetryWin,
								  rcTxtMemDOS;
  TAutoMemoryRc<TStatic*> rcTxtQueueV, rcTxtRetryDOSV, rcTxtRetryWinV;
  TAutoMemoryRc<TGadgetWindow*> rcListTtl;
  TAutoMemoryRc<TEdit*> rcEditAV, rcEditPref, rcEditMask, rcEditPost,
								rcEditMem;

  void near pascal SetPlacement( TWindow *pCtl,
											TWindow *pInd,
											TWindow *pWndRect,
											TWindow *pWndPos
										 );

  void near pascal AcqureListViewer( TListBox* pLBox, TGadgetWindow* pSbar );
  void near pascal AddLay( TWindow *pWnd );
  void near pascal GetFontInfo( TEXTMETRIC& tm );
  void near pascal UpdateLbls( TSlider* pSld );
  void near pascal LoadField( int iInd );
  void near pascal CheckWin( int iDrive, bool bFlCheck );
  void near pascal CheckDOS( int iDrive, bool bFlCheck );
  void near pascal DetachAV( int iIndDta, TIDblListImpDel<TAVDescr>& liDisks );
  void near pascal SelectAndRefresh( int iInd );
  void near pascal TestAndSwitch( int iDrive, int id );
  bool near pascal UpdateInternal( int iIndDta );

  void near pascal RemoveNullDrives( TIDblListImpDel<TAVDescr>& rLi );
  void near pascal ClearChain( int );
  void near pascal AddChain( TIDblListImpDel<TAVDescr>&, int, int, int );

  TAutoMemoryRc<TBrush*> rcBrush;
  HBRUSH EvCtlColor( HDC hDC, HWND hWndChild, uint ctlType );
  void  EvSysColorChange();
  void Ev_CBN_Select();
  void CreateBrush();

  void  UpdateQueue( UINT = 0 );
  void  UpdateRDOS( UINT = 0 );
  void  UpdateRWin( UINT = 0 );

  void  UpdateSld( TSlider*, TStatic*, uint16 );

  DECLARE_RESPONSE_TABLE( TOptionsDialog );
 };

#endif

