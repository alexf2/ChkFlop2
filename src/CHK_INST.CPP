#include <windows.h>
#include <mem.h>
#include <string.h>

#include "chk_inst.hpp"
#include "synth.hpp"
#include "inidta.hpp"

static TRMI rmRmi;
extern "C" int RealIntSimulate( TRMI far*, int iBX, int iCX );

const char far* pPs = "CheckFlop 2.0a";

unsigned int CheckInstall()
 {                            //OUT:ds:si
	memset( &rmRmi, 0 , sizeof(TRMI) );
	rmRmi.EAX = uiASK_CODE;
	RealIntSimulate( &rmRmi, 0x2F, 0 );

	if( LOWORD(rmRmi.EAX) != uiASK_CODE_R  ) return 0;

	TSynthSelector sSel( FlatAddr(rmRmi.DS, rmRmi.ESI), 0xFFFF );

	unsigned int res = strncmp( pPs, (char far*)sSel, strlen(pPs) );

	return !res;
 }
