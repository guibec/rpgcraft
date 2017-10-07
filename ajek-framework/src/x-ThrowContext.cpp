
#include "x-ThrowContext.h"
#include "x-assertion.h"

xThrowContext  g_ThrowCtx;

void xThrowContext::PrintLastError() const
{
	if (!error_msg.IsEmpty()) {
		xPrintLn(error_msg);
	}
}

void xThrowContext::Try(jmp_buf& jbuf)
{
	bug_on_qa(jmp_buffer);		// prev catch already registered

	jmp_buffer			= &jbuf;
	error_module_code	= 0;
	error_info			= 0;
	error_msg.Clear();
}

void xThrowContext::Throw(xThrowModuleCode module_code, const xString& msg)
{
	bug_on_qa(module_code == 0);			// module code must never be 0!
	bug_on_qa(!jmp_buffer);
	if (!msg.IsEmpty()) { error_msg = msg; }
	longjmp(*jmp_buffer, module_code);
}

void xThrowContext::Caught()
{
	// provided for future feature extension to support nested try/catch blocks.
	// for now, NOP out the jmp_buf ptr to indiciate that no catch context is valid...
	// Over values are not cleared to make for easier debug inspection from IDE.

	jmp_buffer = nullptr;
}
