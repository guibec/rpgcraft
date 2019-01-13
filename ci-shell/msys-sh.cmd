@echo off

:: invokes MSYS2 shell directly, not relying on PATH.  This allows the env PATH to preference the Git for Windows
:: instance of Bash/CoreUtils (which typically integrate better with Windows and may also be expected behavior by other
:: build jobs).  MSYS2 is only needed for executing autoconf and makefile things.

SETLOCAL ENABLEDELAYEDEXPANSION

set CHERE_INVOKING=1
set MSYSTEM=MINGW64
set MSYS2_PATH_TYPE=inherit

:: SHLVL is used by bash to decide if it should clear the screen after logout, which fails with an error on
:: jenkins since there is no terminal bound to the shell. This works around it while retaining support for
:: nested shell calls.

IF not defined SHLVL set SHLVL=0
set /a SHLVL+=1
c:\msys64\usr\bin\bash.exe --login %*

ENDLOCAL
