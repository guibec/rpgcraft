#!/bin/bash

me=$(basename "$0")
if [[ -z "$1" || "$1" == "--help" ]]; then
    echo "Converts all spaces to tabs in a given directory."
    echo "Modifies *.cpp;*.h;*.inl;*.lua (tabsize=4)"
    echo "  ex:  $ $me [search_dir]"
    exit -1
fi


function join_by { local IFS="$1"; shift; echo "$*"; }

filetypes=(
    "-name '*.cpp' "
    "-name '*.h'   "
    "-name '*.inl' "
    "-name '*.lua' "
    "-name '*.sh'  "
    "-name '*.fx'  "
)

excludes=(
    "-path '*imports/*'"
)

find . -type f \( $(join_by "-o " ${filetypes[@]} \) -not \( $(join_by "-o " ${excludes[@]} \) -exec bash -c 'unexpand -a -t 4 "$0" > /tmp/e && mv  /tmp/e "$0"' {} \;
