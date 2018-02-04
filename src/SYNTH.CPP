#include "synth.hpp"

static WORD near pascal SynthSelector( DWORD dwLinearAddress, DWORD dwLength )
 {
	WORD tempSelector, selector = NULL;

	tempSelector = AllocSelector( _DS );

	if (tempSelector)
	 {
		SetSelectorBase(tempSelector, dwLinearAddress);
		SetSelectorLimit(tempSelector, dwLength);

		selector = AllocSelector(tempSelector);

		SetSelectorLimit(tempSelector, 100L);
		FreeSelector(tempSelector);
	}
	return selector;
 }

static LPSTR near pascal GetSelectorPointer(WORD selector)
 {
	return (LPSTR)MAKELONG(0, selector);
 }

TSynthSelector::TSynthSelector( DWORD dwLinearAddress, DWORD dwLength )
 {
	wSel = SynthSelector( dwLinearAddress, dwLength );
	bAlloc = 1;
	ptr = GetSelectorPointer( wSel );
 }

TSynthSelector::~TSynthSelector()
 {
	if( bAlloc )
	 {
		bAlloc = 0;
		FreeSelector( wSel );
	 }
 }

TSynthSelector::operator~()
 {
   bAlloc = 0;
 }

TSynthSelector& TSynthSelector::operator=( TSynthSelector& rSel )
 {
	wSel = rSel.wSel;
	ptr = rSel.ptr;
	bAlloc = 1;
	~rSel;

	return *this;
 }

