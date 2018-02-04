#include <windows.h>

void extern TEST( int a );

int PASCAL WinMain( HINSTANCE hCurInstance, HINSTANCE
						  hPrevInstance, LPSTR lpCmdLine, int nCmdShow
						)
 {

	HINSTANCE h = LoadLibrary( "chk_drv.dll" );
	//TEST( 1 );
	FreeLibrary( h );

	return 0;
 }
