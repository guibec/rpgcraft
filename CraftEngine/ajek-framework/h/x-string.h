
#pragma once

#include "x-stl.h"

#if defined(_MSC_VER) && (_DLL == 1) && (_HAS_EXCEPTIONS == 0)
    // When using the MSVCRT DLLs (_DLL==1) with CPP exception support disabled (_HAS_EXCEPTIONS==0), the
    // headers generate superfluous warnings.  This is amusing since the specific code generating the
    // warnings are std::exception classes, which aren't especially useful when exceptions are disabled.
#   pragma warning(disable: 4275)
#endif

#include <string>
#include <cstdarg>      // for va_list

#ifdef _MSC_VER
#   pragma warning(default: 4275)
#endif

#if TARGET_LINUX
#include <cstdio>

// Note: vsnprintf and etc... aren't the same as msvc's vsprintf_s versions!
// msvc's *_s versions will always append a '\0' at the end of the string, whereas
// gcc's vsn* versions will only append a '\0' if it fits within the 'count' field!
// So to fix this problem, we just always append '\0' as the last character in the
// string buffer, which will guarantee the string always ends with a null-byte.
// This should make the functions interchangeable... (unless you are relying on
// the end of the string buffer to not be modified at-all when the formatted string
// is smaller than the buffer... but in that case you should've specified a smaller
// 'size' limit :p) -ncardell
//
// Addendum: Actually, POSIX 2007 changed the rules for vsnprintf such that it DOES
//    behave the same as MSVC's *_s versions.  I confirmed this change is present in
//    the glibc associated with GCC 4.0 and newer (and possibly earlier, though I didn't
//    care to check that far back into pre-history).  But since there's no harm in spamming
//    redundant /0's on infrequently used vsnprinf calls, and since 'fortify' gives us a
//    hard time about it, might as well leave it be.
//      -- jstine

inline __ai int vsprintf_s( char* dest, size_t size, const char* fmt, va_list list )
{
    int ret = vsnprintf( dest, size, fmt, list );
    if (size) dest[size-1] = '\0';
    return ret;
}

template< size_t size >
inline __ai int vsprintf_s(  char (&dest)[size], const char* fmt, va_list list )
{
    int ret = vsnprintf( dest, size, fmt, list );
    if (size) dest[size-1] = '\0';
    return ret;
}

inline int sprintf_s( char* dest, size_t size, const char* fmt, ... )
{
    va_list list;
    va_start(list, fmt);
    int result = vsnprintf( dest, size, fmt, list );
    va_end(list);
    if (size) dest[size-1] = '\0';
    return result;
}

template< size_t size >
inline int sprintf_s(  char (&dest)[size], const char* fmt, ... )
{
    va_list list;
    va_start(list, fmt);
    int result = vsnprintf( dest, size, fmt, list );
    va_end(list);
    if (size) dest[size-1] = '\0';
    return result;
}
#endif

// And nothing rolls this version yet, which allows the use of redefinable pointers
// with fixed lengths, declared as:
// char (*buffer)[length] = nullptr;   // reassignable with new/malloc, VirtualAlloc/mmap, etc.
//
template< size_t size >
inline int vsprintf_s(  char (*&dest)[size], const char* fmt, va_list list )
{
    return vsprintf_s( *dest, fmt, list );
}

template< size_t size >
inline int sprintf_s(  char (*&dest)[size], const char* fmt, ... )
{
    va_list list;
    va_start(list, fmt);
    int result = vsprintf_s( dest, fmt, list );
    va_end(list);
    return result;
}

typedef char            tChar;
typedef std::string     tString;

#define xFmtStr(...)            (xString().Format( __VA_ARGS__ ))
#define cFmtStr(...)            (xString().Format( __VA_ARGS__ ).c_str())
#define cPtrStr(value, ...)     (xPtrStr        ( value, ## __VA_ARGS__ )   .c_str())
#define cHexStr(value, ...)     (xHexStr        ( value, ## __VA_ARGS__ )   .c_str())
#define cDataStr(value)         (xDataStr       ( value )                   .c_str())
#define cDataAsciiStr(value)    (xDataAsciiStr  ( value )                   .c_str())
#define cDecStr(value)          (xDecStr        ( value )                   .c_str())
#define cPosixErrorStr(...)     (xPosixErrorStr ( __VA_ARGS__ )             .c_str())

// semi-internal-use struct for efficiently accepting either xString or `const char*` as input
// parameters to a function.  For use in functions where the majority of use cases involve
// const char* parameter input, such that using const xString& would result in unnecessary
// heap allocations and cleanup operations.

struct qstringlen
{
    qstringlen(const char* src);

    qstringlen(const char* _strptr, int _len)
    {
        strptr = _strptr;
        length = _len;
    }

    const char* strptr;
    int         length;
};

// --------------------------------------------------------------------------------------
//  xString  (definition)
// --------------------------------------------------------------------------------------
// An interface for std::string which provides implicit wide string conversion features,
// and a modern API similar to that of C#/Java.
//
// Remarks:
//   Some methods that act on case sensitivity issues will not produce meaningful results
//   on non-ASCII strings; such as ToUpper ToLower, and  EqualsNoCase.
//
// Implementation notes:
//   This class is intended to be as simple as possible as per allowances made by modern
//   Unicode standards.  As such all strings are assumed to be either UTF8 or UTF16
//   encoding.  Other types of encodings will have to be handled manually before feeding
//   the result to this class.
//
//   UTF16 encoding conversions are supported only on Windows platforms, which prefer UFT16
//   for operating system calls.  Unix platforms should be able to use UTF8 for everything,
//   negating the need to support to convert to/from UTF16.
//
class xString
{
protected:
    std::string     m_string;

public:
    static size_t   npos;

public:
    virtual ~xString() throw() {}

    xString() {}

    xString(const char* src)
        : m_string( src ) { }

    xString(const lua_string& lua_str);

#if TARGET_MSW
    xString( const wchar_t* src )
    {
        // Use the built in UTF conversion stuff.
        // This is supported on all modern windows and linux runtimes, but may not work on
        // especially elderly systems.  Let's hope we never have to deal with any!

        Format( "%S", src );
    }
#endif

     xString&   FormatV         ( const char*    fmt, va_list list );
     xString&   AppendFmtV      ( const char*    fmt, va_list list );
     xString&   Format          ( const char*    fmt=nullptr, ... )     __verify_fmt(2,3);
     xString&   AppendFmt       ( const char*    fmt=nullptr, ... )     __verify_fmt(2,3);

#if TARGET_MSW
     xString&   FormatV         ( const wchar_t* fmt, va_list list );
     xString&   AppendFmtV      ( const wchar_t* fmt, va_list list );
     xString&   Format          ( const wchar_t* fmt, ... )     __verify_fmt(2,3);
     xString&   AppendFmt       ( const wchar_t* fmt, ... )     __verify_fmt(2,3);
#endif

     void       LowercaseInPlace();
     void       UppercaseInPlace();
     xString    ToLower         ()  const;
     xString    ToUpper         ()  const;
     bool       EqualsNoCase    ( const xString& src ) const;
     bool       EqualsAsFilename( const xString& src ) const;
     bool       StartsWith      ( const xString& src ) const;
     bool       StartsWith      ( char c ) const;
     bool       EndsWith        ( const xString& src ) const;
     bool       EndsWith        ( char c ) const;

     xString    GetTail         ( size_t start ) const;
     xString    GetSubstring    ( size_t start, size_t len ) const;
     xString    RemoveAll       ( char c ) const;
     void       RemoveAllInPlace( char c );

     size_t     FindFirst       ( const xString& delims, size_t startpos ) const;
     size_t     FindFirstNot    ( const xString& delims, size_t startpos ) const;
     size_t     FindLast        ( const xString& delims, size_t offset=npos ) const;
     size_t     FindLastNot     ( const xString& delims, size_t offset=npos ) const;

     size_t     FindFirst       ( char c, size_t startpos ) const;
     size_t     FindFirstNot    ( char c, size_t startpos ) const;
     size_t     FindLast        ( char c, size_t offset=npos ) const;
     size_t     FindLastNot     ( char c, size_t offset=npos ) const;

    __ai size_t         Find        ( const xString& str, size_t pos=0 ) const  { return m_string.find(str.m_string, pos); }

    __ai void           Erase       ( size_t startpos, size_t endpos=npos ) { m_string.erase( startpos, endpos ); }
    __ai void           Clear       ()                              { m_string.clear();                 }
    __ai void           Reserve     ( size_t rsv_size )             { m_string.reserve(rsv_size);       }
    __ai void           Resize      ( size_t newsize )              { m_string.resize(newsize);         }
    __ai bool           IsEmpty     ()  const                       { return m_string.empty();          }
    __ai size_t         GetLength   ()  const                       { return m_string.length();         }
    __ai int            Compare     (const xString& right)  const   { return m_string.compare(right);   }

    __ai const char*    c_str       ()  const                       { return m_string.c_str();          }
    __ai xString&       Append      ( const char* src )             { m_string.append( src );       return *this; }
    __ai xString&       Append      ( const char* src, int len )    { m_string.append( src, len );  return *this; }
    __ai xString&       Append      ( char src )                    { m_string += src;              return *this; }
    __ai xString&       PopBack     ()                              { m_string.pop_back();          return *this; }

    // non-const data member is a C++17 addition.  For now it's entirely equivalent to const_cast it.
    __ai const  char*   data        ()  const                       { return m_string.data();           }
    __ai        char*   data        ()                              { return const_cast<char*>(m_string.data());    }

    __ai xString&       Replace     ( size_t pos1, size_t n1, const xString& src)   { m_string.replace(pos1, n1, src.m_string); return *this; }
    __ai const std::string& getBaseType() const                     { return m_string; }

         xString&       operator=   ( const lua_string& src );
    __ai xString&       operator=   ( const char* src )             { if (!src) m_string.clear(); else m_string = src;  return *this;   }
    __ai xString&       operator+=  ( const xString& src )          { Append(src);      return *this;   }
    __ai xString&       operator+=  ( char src )                    { Append(src);      return *this;   }
    __ai tChar&         operator[]  ( size_t idx )                  { return m_string[idx]; }
    __ai const tChar&   operator[]  ( size_t idx ) const            { return m_string[idx]; }
    __ai bool           operator==  ( const xString& right ) const  { return m_string == right.m_string; }
    __ai bool           operator!=  ( const xString& right ) const  { return m_string != right.m_string; }
    __ai bool           operator<   ( const xString& right ) const  { return m_string <  right.m_string; }

    __ai bool           operator==  ( const char* right ) const     { return m_string == right; }
    __ai bool           operator!=  ( const char* right ) const     { return m_string != right; }

    __ai operator const char*() const { return m_string.c_str(); }
    __ai operator qstringlen()  const { return qstringlen(m_string.c_str(), m_string.length() ); }
};


inline __ai xString operator+( const xString& right, const char* src )
{
    return xString(right).Append( src );
}

//inline __ai xString operator+( const char* left, const xString& right )
//{
//  return xString(left).Append( right );
//}


// --------------------------------------------------------------------------------------
//  xStringTokenizer  (class)
// --------------------------------------------------------------------------------------
class xStringTokenizer
{
protected:
    xString         m_string;
    xString         m_delims;
    size_t          m_pos;
    char            m_lastDelim;
    bool            m_IgnoreEmpties;

public:
    xStringTokenizer( const xString& delims, const xString& src=xString() )
        : m_string( src )
        , m_delims( delims )
    {
        m_pos           = 0;
        m_lastDelim     = 0;
        m_IgnoreEmpties = 0;
    }

    xStringTokenizer& SplitString( const xString& src )
    {
        m_string    = src;
        m_pos       = 0;
        m_lastDelim = 0;

        return *this;
    }

    void        IgnoreEmpties   (bool yesOrNo=true)         { m_IgnoreEmpties = yesOrNo; }
    void        SetDelimiters   (const xString& delims)     { m_delims = delims; }

    xString     GetNextToken    ();
    bool        HasMoreTokens   () const;
    char        GetLastDelim    () const                    { return m_lastDelim; }
};


// --------------------------------------------------------------------------------------
//  toUTF16 / toUTF8  (classes)
// --------------------------------------------------------------------------------------
// toUTF16 is available for Windows platforms only, since there's no logical reason
// to use UTF16 on unix platforms.
//

#if TARGET_MSW
class toUTF16
{
protected:
    std::wstring    m_result;

public:
    toUTF16( const xString& src );
    toUTF16( const char* src );

    __ai const wchar_t* wc_str() const { return m_result.c_str(); }
};
#endif

class toUTF8
{
protected:
    xString     m_result;

public:
    toUTF8( const wchar_t* src );

    __ai       char* data()         { return m_result.data (); }
    __ai const char* c_str() const  { return m_result.c_str(); }
};

//
// extern_tmpl  -- MSVC-specific hack necessary for reliable inlining.
//
// Explained: MSVC needs 'extern' on prototypes to allow the use of __forceinline, which implicitly
//            applies 'static' when used.  GCC and the C++ standard, however, forbid the use of
//            extern on template instanizers (its *supposed* to be implied).  MSVC fail.  --jstine
//
#ifdef _MSC_VER
#   define      extern_tmpl     extern
#else
#   define      extern_tmpl
#endif

extern  xString     xBaseFilename       (const xString& src);
extern  xString     xBasePath           (const xString& src);
extern  bool        xIsPathSeparator    (const char& c);
extern  bool        xPathIsAbsolute     (const xString& src);
extern  xString     xPath_Combine       (const xString& left, const xString& right);
extern  xString     xPath_Combine       (const xString& left, const xString& right1, const xString& right2);

extern  xString     xPosixErrorStr      (int errorval);
extern  xString     xPosixErrorStr      ();

extern  bool        xStringToBoolean    (const xString& src);
extern  bool        xStringIsBoolean    (const xString& src);

// --------------------------------------------------------------------------------------
//  xHexStr (template function)
// --------------------------------------------------------------------------------------
// Converts provided integer value into neatly-formatted hex strings.
// 'sep' parameter:  specifies the separator used between each 8-digit block; the default
//                   is underscore '_'.
// If you want to swap the byte order displayed, use xHexStr(xSWAP(val)), or consider using
// xDataStr() instead.

template< typename T >
xString xHexStr( const T& src, const char* sep="_" )
{
    static_assert(std::is_pod<T>::value && !std::is_pod<T>::value,
        "Invalid or unsupported template parameter to xHexStr()"
    );
    src.Invalid_Template_Parameter();       // should generate a compiler error! :)
    return xString();
}

extern  xString xHexStr     ( const u8& src,    const char* sep="_" );
extern  xString xHexStr     ( const u16& src,   const char* sep="_" );
extern  xString xHexStr     ( const u32& src,   const char* sep="_" );
extern  xString xHexStr     ( const s32& src,   const char* sep="_" );
extern  xString xHexStr     ( const u64& src,   const char* sep="_" );
extern  xString xHexStr     ( const s64& src,   const char* sep="_" );
extern  xString xHexStr     ( const u128& src,  const char* sep="_" );


// --------------------------------------------------------------------------------------
//  xDataStr (template function)
// --------------------------------------------------------------------------------------
// Converts provided 128-bit data into a data dump format.  Default separator is SPACE(' '),
// and data is ordered according to in-memory order (LSB->MSB) to better represent the idea of
// a stream of data.
//  * 32-bit  : displays in arrays of 8 bits. (%08x times 1)
//  * 64-bit  : displays in arrays of 32 bits. (%08x times 2)
//  * 128-bit : displays in arrays of 32 bits. (%08x times 4)
//

template< typename T >
xString xDataStr( const T& src )
{
    static_assert(std::is_pod<T>::value && !std::is_pod<T>::value,
        "Invalid or unsupported template parameter to xDataStr()"
    );

    src.Invalid_Template_Parameter();       // should generate a compiler error! :)
    return xString();
}

extern  xString xDataStr        ( const u32& src );
extern  xString xDataStr        ( const u64& src );
extern  xString xDataStr        ( const s64& src );
extern  xString xDataStr        ( const u128& src );

extern  xString xDataAsciiStr( const void* src, size_t length );

// ----------------------------------------------------------------------------
template< typename T >
inline xString xDataAsciiStr( const T& src )
{
    return xDataAsciiStr( &src, sizeof(src) );
}


// --------------------------------------------------------------------------------------
//  xDecStr (template function)
// --------------------------------------------------------------------------------------
// Converts provided integer value into neatly-formatted decimal strings.
// Needed because there's no universal printf formatter for 64 bit values, in particular
// that is friendly between 32 and 64 bit builds.
//

template< typename T >
xString xDecStr( const T& src )
{
    static_assert(std::is_pod<T>::value && !std::is_pod<T>::value,
        "Invalid or unsupported template parameter to xDecStr()"
    );
    src.Invalid_Template_Parameter();       // should generate a compiler error! :)
    return xString();
}

extern  xString xDecStr     ( const u8& src );
extern  xString xDecStr     ( const s8& src );
extern  xString xDecStr     ( const u16& src );
extern  xString xDecStr     ( const s16& src );
extern  xString xDecStr     ( const u32& src );
extern  xString xDecStr     ( const s32& src );
extern  xString xDecStr     ( const u64& src );
extern  xString xDecStr     ( const s64& src );

#if !defined(_MSC_VER)
extern  xString xDecStr     ( const long long& src );
#endif

//extern xString xDecStr    ( const u128& src,  const char* sep );

extern  char*   sbinary (u32 val);
extern  xString xPtrStr (const void*    src, const char* sep=":");
extern  xString xPtrStr (VoidFunc*      src, const char* sep=":");
