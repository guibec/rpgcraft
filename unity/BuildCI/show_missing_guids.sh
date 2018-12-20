#!/bin/bash

function finish {
    [ -n "$scratch" ] && rm -rf $scratch
}

assets_dir=${1:-.}/Assets

if [[ ! -d "$assets_dir" ]]; then
	>&2 echo "The directory '$assets_dir' does not exist."
	>&2 echo "Please run this script from your unity project dir, or specify the dir"
	>&2 echo "on the command line.  Ex:"
	>&2 echo "   $ show_missing_guids.sh /path/to/UnityProject"
fi

# create temp file in shared memory area
scratch=$(mktemp -d -p /dev/shm || exit)
trap finish EXIT

egrep -o "guid:\s*[0-9|a-f]{32}" $(find "$assets_dir" -iname "*.prefab" -o -iname "*.mat") | cut -d':' -f3 | tr -d ' ' | sort | uniq > $scratch/references.txt
find -iname "*.cs.meta" | xargs grep "guid: " | tr -d ' ' | cut -d':' -f3 | sort | uniq > $scratch/metas.txt

# E and F are internal GUIDs.  Probably anything matching this pattern from 0->F is internal to unity but,
# for now, let's just filter out the ones we're explicitly seeing...

echo $'\n0000000000000000e000000000000000' >> $scratch/metas.txt
echo $'\n0000000000000000f000000000000000' >> $scratch/metas.txt

# create a list of objects which are listed in references.txt but are _not_ listed in any meta file.
# this is accomplished by doing a clever operation via "uniq -u".  Getting the uniq list of just
# refs+metas would also give ups all unreferenced meta objects.  Add metas to the list a second time
# and uniq will be sure to eliminate _all_ of them, along with any references they also match.

cat $scratch/references.txt $scratch/metas.txt $scratch/metas.txt | sort | uniq -u > $scratch/missing-refs.txt

if [ -s $scratch/missing-refs.txt ]; then
	echo "Following GUIDs are referenced but have no associated meta definitions:"
	printf "   %s\n" $(cat $scratch/missing-refs.txt)
	exit 1
fi

exit 0
