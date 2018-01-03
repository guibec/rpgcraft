#!/bin/bash

# TODO: add proper commandline support
#   -t    to specify tab size override
#   First naked (non-switch) parameter should be the git repository base dir [optional]

ts=${1:-4}

rootpath=$(pwd)
if [[ "$rootpath" == */.git/* || "$rootpath" == */.git ]]; then
    while [[ "$rootpath" == */.git/* || "$rootpath" == */.git ]]; do
        rootpath=$(dirname "$rootpath"); 
    done
    cd "$rootpath"
fi

gitpath="$(git rev-parse --show-toplevel)"

if [[ -z "$gitpath" ]]; then
    >&2 echo "ERROR: CWD is not a valid or recognized GIT directory."
    exit 1
fi

echo "Installing TAB character filter @ $gitpath"

gitpath="$(readlink -f "$gitpath")"
gitpath="$gitpath/.git"

printf "Registering filter.tabspace with tabsize=$ts... "
git config --local filter.tabspace.clean  "expand --tabs=$ts"      || exit -1
git config --local filter.tabspace.smudge "unexpand -a --tabs=$ts" || exit -1
printf "DONE!\n"

attrib_file="$gitpath/info/attributes"

filetypes=(
    "*.py"
    "*.c"
    "*.cpp"
    "*.h"
    "*.hpp"
    "*.inl"
    "*.sh"
    "*.lua"
    "*.bat"
    "*.cmd"
)

echo "Updating $(readlink -f "$attrib_file")..."

for ft in ${filetypes[@]}; do
    # check if entry for this file type already exists, if so then do not append duplicate.
    # this works nicely to allow a user to comment out lines manually in their attributes file,
    # and this script won't modify or override that preference.
    
    printf "    %-8s   --> " "$ft"
    if grep "$ft"  "$attrib_file" | grep "tabspace" > /dev/null; then
        echo "SKIPPED"
    else
        printf "%-8s   filter=tabspace\n" "$ft"  >> "$attrib_file"
        echo "ADDED"
    fi
done

echo "(note: any skipped entries already exist in attributes file)"
