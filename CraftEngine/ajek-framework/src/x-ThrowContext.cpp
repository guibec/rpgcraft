
#include "PCH-framework.h"

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
    bug_on_qa(jmp_buffer);      // prev catch already registered

    jmp_buffer          = &jbuf;
    error_module_code   = 0;
    error_info          = 0;
    error_msg.Clear();
}

void xThrowContext::Throw(xThrowModuleCode module_code, const xString& msg)
{
    bug_on_qa(module_code == 0);            // module code must never be 0!
    bug_on_qa(!jmp_buffer);
    if (!msg.IsEmpty()) { error_msg = msg; }
    longjmp(*jmp_buffer, module_code);
}

void xThrowContext::Throw(xThrowModuleCode module_code, const AssertContextInfoTriad& triad, const xString& msg)
{
    bug_on_qa(module_code == 0);            // module code must never be 0!
    bug_on_qa(!jmp_buffer);
    error_msg = xFmtStr("%s: %s", triad.filepos, (msg.IsEmpty() && triad.cond) ? triad.cond : msg.c_str());
    if (triad.funcname) {
        error_msg += "\n\t... in function: ";
        error_msg += triad.funcname;
    }

    longjmp(*jmp_buffer, module_code);
}
