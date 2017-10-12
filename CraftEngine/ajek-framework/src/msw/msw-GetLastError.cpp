
#include "PCH-msw.h"
#include <share.h>
#include <cstdarg>
#include <io.h>
#include <fcntl.h>


namespace {
	const char* s_ModuleName = nullptr;		// this module name is not relevant to the errors it logs
};


// --------------------------------------------------------------------------------------
void msw_AttachToParentConsole()
{
#if !defined(_CONSOLE)
	return;
#endif

	if( !::AttachConsole( ATTACH_PARENT_PROCESS ) )	return;

	static const WORD MAX_CONSOLE_LINES = 500;

	int hConHandle;
	intptr_t lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;

	// set the screen buffer to be big enough to let us scroll text

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);

	coninfo.dwSize.Y = MAX_CONSOLE_LINES;

	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),

		coninfo.dwSize);

	// redirect unbuffered STDOUT to the console

	lStdHandle = (intptr_t)GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stdout = *fp;
	setvbuf( stdout, NULL, _IONBF, 0 );

	// redirect unbuffered STDIN to the console

	lStdHandle = (intptr_t)GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "r" );
	*stdin = *fp;
	setvbuf( stdin, NULL, _IONBF, 0 );

	// redirect unbuffered STDERR to the console

	lStdHandle = (intptr_t)GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
	fp = _fdopen( hConHandle, "w" );
	*stderr = *fp;
	setvbuf( stderr, NULL, _IONBF, 0 );

	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
	// point to console as well

	std::ios::sync_with_stdio();
}

// --------------------------------------------------------------------------------------
void msw_LogLastError( const char* filepos, const char* funcname, int lastErrorId, const char* fmt, ...  )
{
	const DWORD BUF_LEN = 1024;
	char t_Msg[BUF_LEN];
	if (!FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, lastErrorId, 0, t_Msg, BUF_LEN, 0))
		strcpy_s( t_Msg, "(no text msg available)" );

	xString details;

	if (fmt)
	{
		va_list list;
		va_start( list, fmt );
		details = "\n";
		details.AppendFmtV( fmt, list );
		va_end( list );
	}

	log_host( "%s Windows Error #%d in function '%s': %s%s",
		filepos, lastErrorId, funcname, t_Msg, details.c_str()
	);
}

// --------------------------------------------------------------------------------------
bool msw_AssertLastError( const char* filepos, const char* funcname, int lastErrorId, const char* fmt, ... )
{
	const DWORD BUF_LEN = 1024;
	char t_Msg[BUF_LEN];
	if (!FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, 0, lastErrorId, 0, t_Msg, BUF_LEN, 0))
		strcpy_s( t_Msg, "(no text msg available)" );

	xString details;

	if (fmt)
	{
		va_list list;
		va_start( list, fmt );
		details = "\n";
		details.AppendFmtV( fmt, list );
		va_end( list );
	}

	return xDebugBreak(
		DbgBreakType_Assert, filepos, funcname, NULL, "Windows Error #%d: %s%s", lastErrorId, t_Msg, details.c_str()
	) == assert_break;
}


// ======================================================================================
// Host_RemoveFolder
// ======================================================================================
// for lack of better location for now...

#include <Shobjidl.h>
#include <direct.h>

void xFileDelete( const xString& file )
{
	DeleteFileW( toUTF16(file).wc_str() );
}
