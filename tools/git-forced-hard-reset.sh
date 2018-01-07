#!/bin/bash
#
# Git makes every attempt to be fast and efficient, and this gets in our way when applying new
# smudge/clean filters to a local repository (this includes changes to crlf rules).  This little
# script gets around that by completely removing all tracked files in the repository and then
# executing git checkout, thus ensuring that all files have been smudged as per new rules assignment.
#
#
# Dev Note:  `git ls-files` works from the CWD, so use `git rev-parse --show-toplevel` to get the
# root of the repo.

rootpath=${1:-$(pwd)}
if [[ "$rootpath" == */.git/* || "$rootpath" == */.git ]]; then
    while [[ "$rootpath" == */.git/* || "$rootpath" == */.git ]]; do
        rootpath=$(dirname "$rootpath"); 
    done
    cd "$rootpath"
fi

gitpath=$(git rev-parse --show-toplevel)

if where cygpath > /dev/null 2>&1; then
    gitpath=$(cygpath "$gitpath")
fi

if [[ -z "$gitpath" ]]; then
    >&2 echo "ERROR: CWD is not a valid or recognized GIT directory."
    exit 1
fi

# So what happens sometimes is that a tool, like tortoisegit, might have a lock on a file and
# prevent it from being removed.  In such a case rm will fail for some files.  Not really anything
# we can do about that.  At least a message gets printed to the user so they know something's up,
# and it's up to them to just "try again later."

echo "Forcibly resetting all tracked files."
git ls-files -z | xargs -0 rm ; git checkout -- .
