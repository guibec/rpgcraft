
#include "PCH-framework.h"
#include "x-unipath.h"

/*
 Cross-platform Path Handling
 ------------------------------
 First, we don't make an effort to support internet URIs at this layer, eg RFC 8089.
 We are only interested in supporting the sort of path names suitable for submission to
 ANSI/STD C file libraries.

 On windows, drive letters are essentially a bastardized URI in the form of C:
 Other platforms may offer their own sort of bastardized URI.  On some console or mobile
 platforms specific path root dirs are handled in a special way that acts like a prefix:
     /debug_server/c:\some\dir\file.ini

 To handle these things as simply and reliably as possible, all internal path name APIs
 do the following:

   1. remove custom target platform prefixes during analysis and processing.
        /debug_server/c:\some\path ->  c:\some\path

   2. convert Windows Drive Letter URIs into unix-style paths, in the same fashion as MSYS2.
        c:\some\path  -> /c/some/path


   3. Perform processing universally, looking only for single forward-slash separators.
      Backslash can be treated as an error on windows platforms.  They are valid filename
      characters on unix.

   4. Re-append prefix removed at step 1.

   5. Convert internal unix-style path to windows path only at the point of calling ANSI C
      or STL file libraries.

         /c/some/path  -> c:\some\path

 */

#if TARGET_MSW
static xPathLayout s_pathfmt_for_libc = PathLayout_Msw;
#else
static xPathLayout s_pathfmt_for_libc = PathLayout_Unix;
#endif

// for use by custom platforms (consoles, phones) which might have unique pathname expectations in
// their custom libc, independent of the regular expectation associated with the branded host OS.
void xPathSetLibcLayout(xPathLayout layout)
{
    x_abort_on(int(layout) < 0 || int(layout) >= PathLayout_MAXINTVAL, "Invalid xPathLayout value=%d", layout);
    s_pathfmt_for_libc = layout;
}

static bool IsMswPathSep(char c)
{
    return (c == '\\') || (c == '/');
}

// intended for use on fullpaths which have already had host prefixes removed.
xString xPathConvertFromMsw(const xString& origPath)
{
    xString result;

    // max output length is original length plus drive specifier, eg. /c  (two letters)
    result.Resize(origPath.GetLength() + 2);
    const char* src = origPath.c_str();
          char* dst = &result[0];

    // Typically a conversion from windows to unix style path has a 1:1 length match.
    // The problem occurs when the path isn't rooted, eg.  c:some\dir  vs. c:\some\dir
    // In the former case, windows keeps a CWD for _every_ drive, and there's no sane way
    // to safely encode that as a unix-style path.  The painful option is to get the CWD
    // for the drive letter and paste it in.  That sounds like work!
    //
    // The real problem is that it's a super-dodgy "feature" of windows, with no easy way
    // to get the CWD of the drive without doing some very racy operations with setcwd/getcwd.
    // More importantly, it literally isn't replicated any other operating system, which means
    // it is very difficult to port logic that somehow relies on this feature.  In the interest
    // of cross-platform support, we detect this and throw a hard error rather than try to support it.

    if (isalnum(src[0]) && src[1] == ':') {
        dst[0] = '/';
        dst[1] = tolower(src[0]);

        // early-exit to to allow `c:` -> `/c`
        // this conversion might be useful for internal path parsing and is an unlikely source of user error.

        if (!src[2]) {
            return result;
        }

        x_abort_on (!IsMswPathSep(src[2]),
            "Invalid msw-specific non-rooted path with drive letter: %s\n\n"
            "Non-rooted paths of this type are not supported to non-standard\n"
            "and non-portable nature of the specification.\n",
            origPath
        );

        src  += 2;
        dst  += 2;
    }

    // - a path that starts with a single backslash is always rejected.
    // - a path that starts with a single forward slash is only rejected if it doesn't _look_ like a
    //   drive letter spec.
    //       /c/woombey/to  <-- OK!
    //       /woombey/to    <-- not good.

    elif (src[0] == '\\') {
        if (src[0] == src[1]) {
            // network name URI, don't do anything (regular slash conversion is OK)
        }
        else {
            x_abort( "Invalid path layout: %s\n"
                "Rooted paths without drive specification are not allowed.\n"
                "Please explicitly specify the drive letter in the path.",
                origPath.c_str()
            );
        }
    }
    elif (src[0] == '/') {
        if (src[0] == src[1]) {
            // network name URI, don't do anything (regular slash conversion is OK)
        }
        else {
            // allow format /c or /c/ and nothing else:
            if (!isalnum(src[1]) || (src[2] && src[2] != '/')) {
                x_abort( "Invalid path layout: %s\n"
                    "Rooted paths without drive specification are not allowed.\n"
                    "Please explicitly specify the drive letter in the path.",
                    origPath.c_str()
                );
            }
        }
    }

    // copy rest of the string char for char, replacing '\\' with '/'
    for(; src; ++src, ++dst) {
        dst[0] = (src[0] == '\\') ? '/' : src[0];
    }
    dst[0] = 0;
    return result;
}

// intended for use on fullpaths which have already had host prefixes removed.
xString xPathConvertToMsw(const xString& unix_path)
{
    xString result;

    result.Resize(unix_path.GetLength());
    const char* src = unix_path.c_str();
          char* dst = &result[0];
    if (src[0] == '/' && isalnum(src[1])) {
        dst[0] = toupper(src[1]);
        dst[1] = ':';
        src  += 2;
        dst  += 2;
    }
    // copy rest of the string char for char, replacing '\\' with '/'
    for(; src[0]; ++src, ++dst) {
        dst[0] = (src[0] == '/') ? '\\' : src[0];
    }
    dst[0] = 0;
    return result;
}

// intended for use on fullpaths which have already had host prefixes removed.
xString xPathConvertToLibc(const xString& unix_path)
{
    if (s_pathfmt_for_libc == PathLayout_Unix) {
        return unix_path;
    }

    // assume s_pathfmt_for_libc == PathLayout_Msw ...
    return xPathConvertToMsw(unix_path);
}


bool xPathIsUniversal(const xString& src)
{
#if TARGET_MSW
    // Windows path rules
    //  - look for c:
    //  - look for backslash
    // If neither is present then the path requires no conversion logic.

    if (isalnum(src[0]) && src[1] == ':') return false;
    return (src.FindFirst('\\', 0) == xString::npos);
#else
    return true;
#endif
}

xUniPath xUniPathInit(const xString& src)
{
    // The implementation of this function is sufficient for Desktop PCs, where it is expected that
    // there is only one type of path, and that sub-paths (via custom prefixes) are not in use.
    // Server/Client or ost/Target environments such as consoles or mobile phones will need to re-implement
    // this function and provide prefix and xPathFormat information accordingly, and also validate the
    // path format (where appropriate).

    if (xPathIsUniversal(src)) {
        return { xString(), src };
    }

    // MSW-only: convert path and indicate such in the structure.
    return { xString(), xPathConvertFromMsw(src), PathLayout_Msw };
}

xString xUniPath::asUnixStr() const {
    return m_hostprefix + m_unixpath;
}

xString xUniPath::GetLibcStr(const xString& new_path) const {
    return m_hostprefix + xPathConvertToLibc(new_path.IsEmpty() ? m_unixpath : new_path);
}


bool xPathIsAbsolute(const xUniPath& upath)
{
    auto& src = upath.m_unixpath;
    if (upath.IsEmpty())    return false;
    if (src[0] == '/')      return true;

    return false;
}

xString xBaseFilename( const xUniPath& upath )
{
    auto& src = upath.m_unixpath;
    size_t pos = src.FindLast('/');
    if (pos == xString::npos) return src;
    return upath.GetOriginalLayoutStr(src.GetTail(pos+1));
}

xString xDirectoryName( const xUniPath& upath )
{
    auto& src = upath.m_unixpath;
    size_t pos = src.FindLast('/');
    if (pos == xString::npos) return xString();     // no path separator?  means it's just a filename
    return upath.GetOriginalLayoutStr(src.GetSubstring(0, pos));
}

__exi xString     xBaseFilename       (const xString& src) { return xBaseFilename       (xUniPathInit(src)); }
__exi xString     xDirectoryName      (const xString& src) { return xDirectoryName      (xUniPathInit(src)); }
__exi bool        xPathIsAbsolute     (const xString& src) { return xPathIsAbsolute     (xUniPathInit(src)); }


xString xUniPath::GetOriginalLayoutStr(const xString& new_path) const
{
    return (m_orig_fmt == PathLayout_Unix) ? asUnixStr() : GetLibcStr();
}

