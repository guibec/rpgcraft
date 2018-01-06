#!/bin/bash

# list of file types on which to apply tabspace filtering.
filetypes_as_tabs=(
    "*.py"
    "*.c"
    "*.cpp"
    "*.cs"
    "*.h"
    "*.hpp"
    "*.inl"
    "*.sh"
    "*.lua"
    "*.bat"
    "*.cmd"
    "*.md"
    "*.txt"
)

# following types are edited as spaces locally, even if --edit-as-tabs has been
# specified.  These types are usually  tabspaced as 2 spaces, and editing them 
# as tabs is not helpful to anyone.

filetypes_as_spaces=(
    "*.csproj"
    "*.props"
    "*.vcxproj"
    "*.xml"
)

FILTER_MODE=help
SHOW_HELP=0
FORCE_RESET=0
NO_RESET=0
ts=4

POSITIONAL=()
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -t|--tabsize)
    ts="$2"
    shift 2 # past argument + value
    ;;
    -f|--force)
    FORCE_RESET="1"
    shift
    ;;
    --no-reset)
    NO_RESET="1"
    shift
    ;;
    --filter-input|--edit-as-spaces)
    FILTER_MODE="input"
    shift
    ;;
    --filter-all|--edit-as-tabs)
    FILTER_MODE="all"
    shift
    ;;
    --filter-none|--edit-as-is)
    FILTER_MODE="none"
    shift
    ;;
    --help)
    SHOW_HELP=1
    shift
    ;;
    *)    # unknown option
    POSITIONAL+=("$1") # save it in an array for later
    shift # past argument
    ;;
esac
done

me=$(basename "$0")

if [[ "$SHOW_HELP" -eq "1" || "$FILTER_MODE" == "help" ]]; then
    echo "$me - Sets TabSpace Filtering Rules for GIT"
    echo
    echo "To edit locally as tabs and convert to spaces upstream :"
    echo "  $ $me --tabsize=4 --edit-as-tabs {repository_path}"
    echo
    echo "To edit locally as spaces and convert errant tabs to spaces upstream:"
    echo "  $ $me --tabsize=4 --edit-as-spaces {repository_path}"
    echo
    echo "To disable the filter and restore default  behavior:"
    echo "  $ $me --edit-as-is {repository_path}"
    echo
    echo '  `repository_path` is optional.  If not specified, the GIT repository associated'
    echo '  with the CWD is used.'
    echo
    
    exit 1
fi

set -- "${POSITIONAL[@]}" # restore positional parameters

rootpath=${1:-$(pwd)}
if [[ "$rootpath" == */.git/* || "$rootpath" == */.git ]]; then
    while [[ "$rootpath" == */.git/* || "$rootpath" == */.git ]]; do
        rootpath=$(dirname "$rootpath"); 
    done
    cd "$rootpath"
fi

gitpath=$(git rev-parse --show-toplevel)

# rev-parse returns a malformed filename on Windows OS, which contains both a drive:colon
# specifier and forward slashes, eg c:/some/path -- this confuses msys into not auto-correcting
# the path to the expected bash format of /c/some/path.  So we must do so manually via cygpath,
# elsewise some of our later commands like grep might fail to find the file...

if where cygpath > /dev/null 2>&1; then
    gitpath=$(cygpath "$gitpath")
fi

if [[ -z "$gitpath" ]]; then
    >&2 echo "ERROR: CWD is not a valid or recognized GIT directory."
    exit 1
fi

cd "$gitpath"
if ! git diff-index --quiet HEAD --; then
    if [[ "$FORCE_RESET" -eq "0" && "$NO_RESET" -eq "0" ]]; then
        >&2 echo "ERROR: Local changes detected in the repository."
        >&2 echo "  Specify -f to forcibly reset the repository to the new filter setting."
        >&2 echo "  Specify --no-reset to skip the forcible reset step.  This will probably lead to unwanted"
        >&2 echo "  or undefined behavior when attempting to switch branches and check in modifications"
        >&2 echo "  to stale (unsmudged) files."
        exit 1
    fi
fi

echo "Applying changed to clone @ $gitpath"

gitpath="$(readlink -f "$gitpath")"
gitpath="$gitpath/.git"

if   [[ "$FILTER_MODE" != "none" ]]; then
    printf "Registering filters with tabsize=$ts... "
    git config --local  filter.editastabs.clean   "expand --tabs=$ts"                    || exit -1
    git config --local  filter.editastabs.smudge  "unexpand --tabs=$ts --first-only"     || exit -1

    git config --local  filter.editasspaces.clean  "expand --tabs=$ts"      || exit -1
    git config --local  filter.editasspaces.smudge "cat"                    || exit -1
    printf "DONE!\n"
fi

attrib_file="$gitpath/info/attributes"

echo "Updating $(readlink -f "$attrib_file")..."

# Remove existing entries, stop after one match.
line_first=$(grep -m1 -n "# FilterTab Section:BEGIN" "$attrib_file" 2>/dev/null | cut -d':' -f1)
line_last=$(grep  -m1 -n "# FilterTab Section:END"   "$attrib_file" 2>/dev/null | cut -d':' -f1)

if [[ -n "$line_first" && -n "$line_last" ]]; then
    echo "    Deleting existing FilterTab Section found at lines $line_first -> $line_last"
    sed -i -e "${line_first},${line_last}d" "$attrib_file"
fi

declare -a filetypes

add_filetypes_to_attributes_file() {
    filtername="$1"
    for ft in "${filetypes[@]}"; do
        # check if entry for this file type already exists, if so then do not append duplicate.
        # this works nicely to allow a user to comment out lines manually in their attributes file,
        # and this script won't modify or override that preference.
        
        printf "    %-12s   --> " "$ft"
        if grep "$ft"  "$attrib_file" | egrep "editas\S" > /dev/null 2>/dev/null; then
            echo "SKIPPED"
        else
            >> "$attrib_file" printf "%-12s  filter=$filtername\n" "$ft"
            echo "ADDED as filter=$filtername"
        fi
    done
}

if [[ "$FILTER_MODE" != "none" ]]; then
    >> "$attrib_file" echo "# FilterTab Section:BEGIN"
    >> "$attrib_file" echo "# warning: git-filter-tab-install will overwrite any direct modifications to this section."

    filetypes=("${filetypes_as_tabs[@]}")
    tabulars=$([ "$FILTER_MODE" == "all" ] && echo "editastabs" || echo "editasspaces")
    add_filetypes_to_attributes_file "$tabulars"
    
    filetypes=("${filetypes_as_spaces[@]}")
    add_filetypes_to_attributes_file "editasspaces"

    >> "$attrib_file" echo "# FilterTab Section:END"
    echo "(note: any skipped entries already exist in attributes file)"

else
    echo "FilterTab attributes removed."
fi

if [[ "$NO_RESET" -ne "1" ]]; then
    # git reset --hard isn't enough.  Gotta forcibly remove everything and re-checkout.
    echo "Applying new filter settings (very hard reset)..."
    git ls-files -z | xargs -0 rm ; git checkout -- .
fi