
# msys-sh.cmd

A script which invokes MSYS2 shell directly, not relying on PATH.
MSYS2 is expected to be installed at `c:\msys64`

## All the Details
MSYS2 is a variant of CoreUtils/BASH which comes pre-installed on Appveyor windows slave images.
It supports a package manager called pacman, which can be used to install various bits of the
build-essential toolchain-- autoconf, automake, makefile, etc.

This script is intended to be invoked directly from CI scripts only, such as Jenkinsfile or
Appveyor.yml.  These slave build environments are controlled environments where we can safely
assume something like MSYS2 being installed to `c:\msys64`.

## Caveats

 * Caution! Do not install GIT to msys2.  Build slaves will already have Git for Windos installed,
   and MSYS2 GIT has problems.

 * Take note that unexpected behavior may occur if the build scripts invoke CMD batch scripts
   which in turn invoke Git for Windows BASH scripts.  This typically manifests as a shared DLL
   error but in rare cases may throw other errors.

## Shared DLL Error in Detail

Shared DLL errors may occur if forcibly invoking BASH from Git for Windows from an MSYS2 subshell,
or vice versa.  In normal operation of shell scripts this does not occur, because the parent shell
invocation will add it's personal BASH instance to the PATH.  Therefore any subsequent invocation
of `/bin/bash/` will always run one which matches the shell.

The problem occurs when a CMD Batch (`.bat`) file runs a shell script.  Normally a batch file
written to be runnable by an end user makes the assumption that BASH is not in the path, and it
will instead try to find BASH installed by Git for Windows and run that directly.  This causes the
error.

The fix is to try to run BASH from PATH first, and then fall back on Git for Windows location
detection only if the PATH is not set up.  If any batch file scripts are invoked which do not
check the PATH for BASH before falling back on Git for Windows then these must be fixed directly.

A shell-invocation script called `run-sh.cmd` has been provided separately which is ideal for use
when it is necessary to invoke an MSYS2 script from cmd batch. It does a robust check of PATH, Git
for Windows, and MSYS2 installs.
