#pragma once

#include "x-types.h"
#include "x-string.h"
#include <setjmp.h>

// xThrowModule_Script
//  - Scripts can throw meaningful recoverable errors related to script parsing or script execution.
//
// xThrowModule_Bridge
//  - Bridge or data marshalling errors -- these are not bound to any specific script line of code.
//    Usually thrown due to bad data structures for fixed-layout data required by the engine.
//
// xThrowModule_GPU
//  - GPU can throw meaningful recoverable errors related to shader compilation, input layouts, etc.
//

enum xThrowModuleCode
{
    xThrowModule_Invalid = 0,           // 0 is used by setjmp for "run try block"
    xThrowModule_Script  = 1,
    xThrowModule_Bridge,
    xThrowModule_GPU,
    xThrowModule_FMOD,
};

class xThrowContext
{
    // Dev Note:
    //  It is possible to extend this system to support nested try/catch blocks, if warranted.
    //  Current goal is to design the software so that such nesting is *not* required, and if
    //  used, only done so in a debugging capacity.
    //
    //  To support nesting, turn the jmp_buffer into an array and inc/dec an index to the array
    //  on calls to x_try() / x_catch().  User must manually re-throw from default case in switch,
    //  which will automatically propagate to next handler in the hierarchy.

public:
    jmp_buf*    jmp_buffer          = nullptr;
    int         error_module_code   = 0;
    s64         error_info          = 0;
    xString     error_msg;

public:
    bool            CanThrow        () const    { return jmp_buffer != nullptr; }
    const xString&  GetErrorStr     () const    { return error_msg;             }
    void            Finalize        ()          { jmp_buffer = nullptr;         }

            void    Try             (jmp_buf& buf);

    virtual void    SetErrorInfo    (s64 code)  { error_info = code;            }
    virtual void    Throw           (xThrowModuleCode module_code, const xString& msg=xString());       // noreturn type
    virtual void    Throw           (xThrowModuleCode module_code, const AssertContextInfoTriad& triad, const xString& msg=xString());      // noreturn type
    virtual void    PrintLastError  () const;
};

#define DECLARE_MODULE_THROW(module_code)                               \
    static xThrowContext&   s_ThrowCtxModuleAlias   = g_ThrowCtx;       \
    static xThrowModuleCode s_ThrowCtxModuleCode    = module_code


#define throw_abort_ex(info, ...)               ((s_ThrowCtxModuleAlias.CanThrow() && (s_ThrowCtxModuleAlias.SetErrorInfo(info) ,s_ThrowCtxModuleAlias.Throw(s_ThrowCtxModuleCode, {__FILEPOS__, __FUNCTION_NAME__, nullptr}, xFmtStr(__VA_ARGS__)) , true)) || _inline_abort_(  "throw"          , __VA_ARGS__))
#define throw_abort(...)                        ((s_ThrowCtxModuleAlias.CanThrow() && (                                          s_ThrowCtxModuleAlias.Throw(s_ThrowCtxModuleCode, {__FILEPOS__, __FUNCTION_NAME__, nullptr}, xFmtStr(__VA_ARGS__)) , true)) || _inline_abort_(  "throw"          , __VA_ARGS__))
#define throw_abort_on(cond, ...)   ( (cond) && ((s_ThrowCtxModuleAlias.CanThrow() && (                                          s_ThrowCtxModuleAlias.Throw(s_ThrowCtxModuleCode, {__FILEPOS__, __FUNCTION_NAME__, # cond }, xFmtStr(__VA_ARGS__)) , true)) || _inline_abort_( "(throw)" # cond  , __VA_ARGS__)) )

// x_throw disabled -- all throws should conditionally abort if no catch handler is registered.
//#define x_throw_ex(info, ...)                 ((s_ThrowCtxModuleAlias.CanThrow() && (s_ThrowCtxModuleAlias.SetErrorInfo(info) ,s_ThrowCtxModuleAlias.Throw(s_ThrowCtxModuleCode, {__FILEPOS__, __FUNCTION_NAME__, nullptr}, xFmtStr(__VA_ARGS__)) , true)))
//#define x_throw(info, ...)                    ((s_ThrowCtxModuleAlias.CanThrow() &&                                            s_ThrowCtxModuleAlias.Throw(s_ThrowCtxModuleCode, {__FILEPOS__, __FUNCTION_NAME__, nullptr}, xFmtStr(__VA_ARGS__)) , true)))

struct CatchFinalizer{
    ~CatchFinalizer();
};

#define x_try()                                                             \
    jmp_buf _jmp_buf;                                                       \
    int     _setjmp_result = (g_ThrowCtx.Try(_jmp_buf), setjmp(_jmp_buf));  \
    CatchFinalizer _setjmp_finalizer;                                       \
    if (_setjmp_result == 0)

#define x_catch()                                                           \
    else switch (_setjmp_result)

#define x_finalize() if(1)

#define x_exccode()  (_setjmp_result)

extern xThrowContext    g_ThrowCtx;


inline CatchFinalizer::~CatchFinalizer() {
    g_ThrowCtx.Finalize();
}
