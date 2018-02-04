#if !defined(_TCENTERDLG_)
#define _TCENTERDLG_

#include <owl\owlpch.h>
#include <owl\dialog.h>
#include <classlib\dlistimp.h>

class __rtti TZPlacement
 {
public:
  HWND hwnd;

  TZPlacement( HWND hw ) { hwnd = hw; }
  TZPlacement( TWindow* pW ) { hwnd = pW->HWindow; }
  TZPlacement( TWindow& rW ) { hwnd = rW.HWindow; }

  virtual ~TZPlacement() {};

  int operator==( const TZPlacement& avD ) const
	{
	  return this == &avD;
	}

  unsigned HashValue() const { return 0; }

 };

class __rtti TZGroup: public TZPlacement
 {
public:
  TZGroup( HWND hw ): TZPlacement( hw ) {};
  TZGroup( TWindow* pW ): TZPlacement( pW ) {};
  TZGroup( TWindow& rW ): TZPlacement( rW ) {};
 };

class __rtti TZCtl: public TZPlacement
 {
public:
  TZCtl( HWND hw ): TZPlacement( hw ) {};
  TZCtl( TWindow* pW ): TZPlacement( pW ) {};
  TZCtl( TWindow& rW ): TZPlacement( rW ) {};
 };

class _MYEXPORTCLASS TCenterDialog: public TDialog
 {
public:
  TCenterDialog( TWindow* parent, TResId resId, TModule* module = 0 ):
	 TDialog( parent, resId, module )
	{};
  virtual ~TCenterDialog();

  TCenterDialog& operator+( TZPlacement& rPl )
	{
	  liZOrder.AddAtTail( &rPl );
	  return *this;
	}

  TCenterDialog& operator+( TZPlacement* pPl )
	{
	  return operator+( *pPl );
	}

  void InitZOrder_();

protected:
  virtual void SetupWindow();

private:
  TIDoubleListImp<TZPlacement> liZOrder;
 };

#endif


