#if !defined(_SYNTH_SEL_)
#define _SYNTH_SEL_

#include <windows.h>

struct TRMI
 {
	DWORD EDI;
	DWORD ESI;
	DWORD EBP;
	DWORD reserved_by_system;
	DWORD EBX;
	DWORD EDX;
	DWORD ECX;
	DWORD EAX;
	WORD flags;
	WORD ES,DS,FS,GS,IP,CS,SP,SS;
};


inline DWORD FlatAddr( WORD wSeg, WORD wOffs )
 {
	return  ((DWORD)wSeg << 4L) + (DWORD)wOffs;
 }

class TSynthSelector
 {
public:
  TSynthSelector(): bAlloc( 0 ) {};
  TSynthSelector( DWORD dwLinearAddress, DWORD dwLength );
  ~TSynthSelector();

  TSynthSelector& operator=( TSynthSelector& rSel );
  TSynthSelector( TSynthSelector& rSel )
	{
	  operator=( rSel );
	}

  WORD operator()() { return wSel; }
  operator char far*() { return ptr; }
  operator~();

private:
  WORD wSel;
  char far* ptr;
  int bAlloc;
 };

#endif
