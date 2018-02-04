#if !defined(_EXEC_)
#define _EXEC_

#include <windows.h>
#include <dlgs.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EXEC_ERR 32

typedef struct _EXECAPPINFO {
	 HINSTANCE hInstance;            // The instance value
	 HWND hWnd;                      // The main window handle
	 HTASK hTask;                    // The task handle
} EXECAPPINFO, FAR *LPEXECAPPINFO;


extern UINT ExecApp( HWND hwndParent,
							LPSTR pszName,
							LPSTR pszParams,
							LPEXECAPPINFO pInfo,
							HINSTANCE ghInstance
						 );


extern  BOOL IsAppRunning( LPEXECAPPINFO pInfo );

#endif

