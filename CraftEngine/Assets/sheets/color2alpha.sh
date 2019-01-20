#!/bin/bash
#
# For converting typical opaque 

if [[ -z "$1" || -z "$2" || "$1" == "--help" ]]; then
    >&2 echo "Convert transparent color into GPU-friendly alpha channel layer."
    >&2 echo ""
    >&2 echo "USAGE:"
    >&2 echo "    $ color2alpha im_color srcimg [destimg]"
    >&2 echo ""
    exit 1
fi


color=$1
infile=$2
outfile=${3:-$infile}

# use set -x to echo the command for diag purposes...
( set -x ; magick convert "$infile" -alpha set -channel RGBA -fill none -opaque "$color" "$outfile" )
