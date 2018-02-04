#include "appexec.h"


typedef struct _ENUMINFO {
	 HINSTANCE hInstance;        // supplied on input
	 HWND hWnd;                  // filled in by enum func
} ENUMINFO, FAR *LPENUMINFO;


static BOOL BrowseForApp( HWND hwndParent, LPSTR pszPath, LPSTR pszNewPath );
static int TryWinExec( LPSTR pszPath, LPSTR pszParams, LPEXECAPPINFO pInfo,
							  HINSTANCE ghInstance );
BOOL CALLBACK EnumWndProc( HWND hWnd, LPARAM lParam );


UINT ExecApp( HWND hwndParent,
				  LPSTR pszPath,
				  LPSTR pszParams,
				  LPEXECAPPINFO pInfo,
				  HINSTANCE ghInstance
				)
 {
	 char szPath[_MAX_PATH];
	 char szNewPath[_MAX_PATH];
	 char szDrive[_MAX_DRIVE];
	 char szDir[_MAX_DIR];
	 char szFname[_MAX_FNAME];
	 char szExt[_MAX_EXT];
	 int i;

	 _fstrcpy(szPath, pszPath);
	 _fstrupr(szPath);

	 _splitpath(szPath, szDrive, szDir, szFname, szExt);

	 if( !*szExt ) _fstrcpy( szExt, ".EXE" );
	 //else if( !_fstricmp(szExt, ".EXE") ) return INVALID_NAME;

	 _makepath(szNewPath, szDrive, szDir, szFname, szExt);

	 return TryWinExec( szNewPath, pszParams, pInfo, ghInstance );
 }


static int TryWinExec( LPSTR pszPath, LPSTR pszParams, LPEXECAPPINFO pInfo, HINSTANCE ghInstance )
 {
	 char szCmdLine[_MAX_PATH + 256];
	 UINT uiResult;
	 ENUMINFO EnumInfo;
	 WNDENUMPROC lpEnumProc;

	 _fstrcpy( szCmdLine, pszPath );
	 _fstrcat( szCmdLine, " " );
	 _fstrcat( szCmdLine, pszParams );
	 uiResult = WinExec( szCmdLine, SW_SHOWNORMAL );
            
	 if( uiResult < MAX_EXEC_ERR ) return uiResult;

	 pInfo->hInstance = (HINSTANCE)uiResult;

	 EnumInfo.hInstance = pInfo->hInstance;
	 EnumInfo.hWnd = NULL;

	 lpEnumProc = (WNDENUMPROC)MakeProcInstance( (FARPROC)EnumWndProc,
																ghInstance );
	 EnumWindows( lpEnumProc, (LPARAM)(LPENUMINFO)&EnumInfo );
	 FreeProcInstance( (FARPROC)lpEnumProc );

	 if (!EnumInfo.hWnd)
	  {
		 pInfo->hWnd = NULL;
		 pInfo->hTask = NULL;
	  }
	 else
	  {
		 pInfo->hWnd = EnumInfo.hWnd;
		 pInfo->hTask = GetWindowTask( pInfo->hWnd );
	  }

	 return uiResult;
 }


BOOL CALLBACK EnumWndProc(HWND hWnd, LPARAM lParam)
 {
	HINSTANCE hInstance;
	LPENUMINFO lpInfo;

	lpInfo = (LPENUMINFO)lParam;
	hInstance = (HINSTANCE) GetWindowWord(hWnd, GWW_HINSTANCE);

	if (hInstance == lpInfo->hInstance)
	 {
		lpInfo->hWnd = hWnd;
		return FALSE;
	 }

	return TRUE;
}


BOOL IsAppRunning(LPEXECAPPINFO pInfo)
{
  if( !IsWindow(pInfo->hWnd) || !IsTask(pInfo->hTask) ) return FALSE;

  if( (HINSTANCE)GetWindowWord(pInfo->hWnd,
										 GWW_HINSTANCE) != pInfo->hInstance
		) return FALSE;

  if( GetWindowTask(pInfo->hWnd) != pInfo->hTask ) return FALSE;

  return TRUE;
}


