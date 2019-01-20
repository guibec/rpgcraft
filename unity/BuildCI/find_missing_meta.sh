#!/bin/bash
 
mydir=$(readlink -f $(dirname ${BASH_SOURCE[0]}))
assets_dir=${1:-.}/Assets

if [[ ! -d "$assets_dir" ]]; then
	>&2 echo "The directory '$assets_dir' does not exist."
	>&2 echo "Please run this script from your unity project dir, or specify the dir"
	>&2 echo "on the command line.  Ex:"
	>&2 echo "   $ find_missing_meta.sh /path/to/UnityProject"
	exit 1
fi

echo "Verifying unity metadata-to-assets references..."

# search for any files in the assets dir which don't have associated .meta files.
# done by listing all files, and stripping .meta from .meta files.  Any item in the list
# which is not duplicated will be left-over after "uniq -u"

find_result=$(find "$assets_dir" -iname "*" | sed s/.meta$//)

find_result+=$'\n'"$assets_dir"
result=$(echo "$find_result" | sort | uniq -u)

if [[ -n "$result" ]]; then
	>&2 echo "The following assets are missing required .meta files:"
	>&2 printf "    %s\n" $(echo "$result")
	
	>&2 printf "\n"
	"$mydir/show_missing_guids.sh" "$1"
	exit 1
fi

exit 0
