#pragma once

// x-unixpath structures and prototypes are intended for internal use.
// Feel free to use them when some sort of micro-optimization is needed.

#include "x-types.h"
#include "x-string.h"

enum xPathLayout
{
    PathLayout_Unix =0,     // only forward slashes and NUL are reserved
    PathLayout_Msw,         // forward/backslashes are allowed delimiters, drive letters, colons, lots of special rules, etc.

    PathLayout_MAXINTVAL    // for sanity checks
};

struct xUnixPath
{
    xString     m_hostprefix;       // optional host prefix part, includes trailing slash or colon
    xString     m_unixpath;         // unix-style path (always delimited by forward slash even on windows)
    xPathLayout m_orig_fmt;         // controls how the path is formatted when calling GetOriginalLayoutStr()

    xString     asUnixStr            () const;
    xString     GetLibcStr           (const xString& new_path=xString()) const;
    xString     GetOriginalLayoutStr (const xString& new_path) const;

    bool IsEmpty() const {
        // whether or not the prefix is empty doesn't matter for the purpose of this being an operable path.
        return m_unixpath.IsEmpty();
    }
};

extern  void        xPathSetLibcLayout      (xPathLayout layout);

extern  xUnixPath   xUnixPathInit           (const xString& src);
extern  xString     xBaseFilename           (const xUnixPath& src);
extern  xString     xDirectoryName          (const xUnixPath& src);
extern  bool        xPathIsAbsolute         (const xUnixPath& src);
extern  bool        xCreateDirectory        (const xUnixPath& orig_unix_dir);

extern xString      xPathConvertToUnix      (const xString& origPath);
extern xString      xPathConvertToLibc      (const xString& unix_path);
