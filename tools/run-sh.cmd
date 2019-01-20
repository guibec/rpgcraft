@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

:: Caveats:
::   * bash.exe in the PATH is often "bogus" -- a few apps, chocolately installers in particular, install stripped-down
::     non-functional versions of bash.exe into the PATH just so they can glob files (making them equivalent to malware imo)
::
::   * No way to know where MSYS2 is installed, so impose a requirement that it be c:\msys64\
::
::   * GIT Bash usually sets up an sh_auto_file association that allows sh files to be run directly.  But sometimes it's
::     missing from developers' installs, or has been incorrectly re-assigned to load sh scripts into text files.

:: favor git-bash ahead of MSYS2 bash as it generally integrates better with Windows.
where git 2>NUL 1>NUL
if %errorlevel% == 0 (
    FOR /F "tokens=* USEBACKQ" %%F IN (`where git`) DO (
        SET git_path=%%F
    )
    
    if defined git_path (
        call :dirname result "!git_path!"
        set _bash_path=!result!\..\bin
    )
)

:: falback on MSYS2 bash at the fixed location...
if not defined _bash_path (
    if EXIST c:\msys64\usr\bin\bash.exe (
        set _bash_path=c:\msys64\usr\bin
    )
)

if not defined _bash_path (
    >&2 echo error: GIT bash was not found in the PATH.
    >&2 echo Please install GIT bash and ensure GIT is added to the PATH.
    exit /b 1
)

:: MSYS2 doesn't integrate with cmd shell the same way git-bash does.  It needs some env setup:

if not defined git_path (
    set CHERE_INVOKING=1
    set MSYSTEM=MINGW64
    set MSYS2_PATH_TYPE=inherit
)

:: avoid environment pollution in subshell
set "git_path="

:: SHLVL is used by bash to decide if it should clear the screen after logout, which fails with an error on
:: jenkins since there is no terminal bound to the shell. This works around it while retaining support for
:: nested shell calls.

IF not defined SHLVL set SHLVL=0
set /a SHLVL+=1
"%_bash_path%\bash.exe" --login %*
ENDLOCAL
exit /b %errorlevel%

:dirname <resultVar> <pathVar>
(
    set "%~1=%~dp2"
    exit /b
)