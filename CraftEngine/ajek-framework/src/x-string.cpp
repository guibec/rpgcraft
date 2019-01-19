
#include "PCH-framework.h"

#include "x-types.h"
#include "x-stl.h"
#include "x-string.h"


DECLARE_MODULE_NAME( "x-string" );

//
// Converts a u32 to a binary string.
// [TODO] : Rename this to "binaryToString()" or something slightly more obvious.
//          I doubt a longer name will be inhibiting given the general lack of use
//          this function will see in daily coding.
//
char* sbinary(u32 val)
{
    static char bchars[64];     // oh the multithreaded hell to pay.

    char *c_p = &bchars[0];

    for(int n=0; n!=32; n++)
    {
        char c= ( (s32)val >= 0) ? '0' : '1' ;
        val=val<<1;
        *c_p++=c;
    }
    *c_p++=0;
    return  &bchars[0];
}


qstringlen::qstringlen(const char* src) {
    strptr = src;
    length = strlen(src);
}


// ----------------------------------------------------------------------------
//  toUTF16 / toUTF8  (implementations)
// ----------------------------------------------------------------------------
// toUTF16 is available for Windows platforms only, since there's no logical reason
// to use UTF16 on unix platforms.
//
#if TARGET_MSW
toUTF16::toUTF16( const xString& src )
{
    // converting to UTF16 is easy -- resulting string length is never more than
    // the length of the incoming string.

    size_t newlen = src.GetLength() + 4;

    wchar_t* dest = (wchar_t*)malloc( newlen * sizeof(wchar_t) );
    swprintf_s( dest, newlen, L"%S", src.c_str() );
    m_result.assign( dest );
}

toUTF16::toUTF16( const char* src )
{
    // converting to UTF16 is easy -- resulting string length is never more than
    // the length of the incoming string.

    size_t newlen = strlen(src) + 4;

    wchar_t* dest = (wchar_t*)malloc( newlen * sizeof(wchar_t) );
    swprintf_s( dest, newlen, L"%S", src );
    m_result.assign( dest );
}
#endif

toUTF8::toUTF8( const wchar_t* src )
{
    // converting to UTF8 is tricky -- resulting string length can be much longer than
    // the source if the source contains a number of non-ASCII characters.  Fortunately
    // xString::Format does buffer adjustments for us.

    m_result.Format( "%S", src );
}

// ----------------------------------------------------------------------------
//  xstring_format_buffer  (struct)
// ----------------------------------------------------------------------------
// The stack-allocated destination buffer is used in most cases.
// This struct only falls back on heap allocations for especially long strings.
//
template< typename T >
struct xstring_format_buffer
{
    T   fast[2048];
    T*  heap_alloc;
    T*  ptr;

    xstring_format_buffer()
    {
        heap_alloc  = nullptr;
        ptr         = fast;
    }

    ~xstring_format_buffer() throw()
    {
        free( heap_alloc );
    }

    void Alloc( size_t newsize )
    {
        free( heap_alloc );
        ptr = heap_alloc = (T*)malloc( newsize * sizeof(T) );
    }
};


// ----------------------------------------------------------------------------
//  xString  (implementations)
// ----------------------------------------------------------------------------
// Wide versions are currently availabe only on Windows (MSW) platforms, where the OS
// wants or requires some strings to be in UTF16 form.
//

#if !TARGET_MSW
__ai size_t _vscprintf( const char* fmt, va_list args )
{
    // glibc implicitly passes va_list by reference rather than by value, which
    // appears to be a C99-ish thing (though the standard, in fact, is open to
    // allowing either since C89).  C99 introduces va_copy(), which is needed to
    // prevent the inner function from clobbering the by-reference parameter
    // for the caller.  --jstine

    va_list l2;
    va_copy(l2, args);
    auto result = vsnprintf( nullptr, 0, fmt, l2);
    va_end(l2);
    return result;
}
#endif

#if TARGET_MSW
static void _format_raw( xstring_format_buffer<wchar_t>& dest, const wchar_t* fmt, va_list list )
{
    int destSize = _vscwprintf( fmt, list ); // _vscprintf doesn't count terminating '\0'
    bug_on_qa( destSize < 0, "Invalid string formatting parameters!" );
    ++destSize;

    if (destSize > bulkof(dest.fast)) {
        dest.Alloc( destSize );
    }

    vswprintf_s( dest.ptr, destSize, fmt, list );
}
#endif

size_t xString::npos = std::string::npos;

// --------------------------------------------------------------------------------------
// Performs case-insensitive compare.
// Completely disregards UTF specifications so don't use this on anything except ascii-based
// dev/debug input string comparisons.  If doing filename comparisons, use EqualsAsFilename
// instead.
bool xString::EqualsNoCase( const xString& src ) const
{
    size_t len1 = GetLength();
    size_t len2 = src.GetLength();

    if( len1 != len2 ) return false;

    for( size_t i=0; i<len1; ++i )
    {
        if(tolower(m_string[i]) != tolower(src[i]))
            return false;
    }

    return true;
}

bool xString::EqualsAsFilename( const xString& src ) const
{
#if TARGET_MSW
    // in order to do reliable UTF8 support we need to convert the string to UTF16
    // and use windows built-in compares.  Standard tolower/toupper won't get a reliable
    // result on some UTF8 strings.

    int length = GetLength();
    if (src.GetLength() != length) return false;

    int result = _wcsnicmp( toUTF16(*this).wc_str(), toUTF16(src).wc_str(), length );
    return result == 0;

#else
    return m_string == src.m_string;
#endif
}

bool xString::StartsWith( const xString& src ) const
{
    return m_string.compare( 0, src.GetLength(), src ) == 0;
}

bool xString::StartsWith( char c ) const
{
    return GetLength() && (m_string[0] == c);
}

bool xString::EndsWith( const xString& src ) const
{
    int startpos = m_string.length()-src.GetLength();
    if (startpos<0) return false;
    return m_string.compare( startpos, src.GetLength(), src ) == 0;
}

bool xString::EndsWith( char c ) const
{
    if (m_string.empty()) return false;
    return m_string[m_string.length()-1] == c;
}

xString xString::GetTail( size_t start ) const
{
    xString result;
    if (start < m_string.length()) {  // to avoid STL error/exception mess...
        result.m_string = m_string.substr( start, npos );
    }
    return result;
}

xString xString::GetSubstring( size_t start, size_t len ) const
{
    xString result;
    result.m_string = m_string.substr( start, len );
    return result;
}

size_t xString::FindFirst   (char c,                    size_t startpos ) const { return m_string.find_first_of     ( c,        startpos ); }
size_t xString::FindFirst   (const xString& delims,     size_t startpos ) const { return m_string.find_first_of     ( delims,   startpos ); }
size_t xString::FindFirstNot(char c,                    size_t startpos ) const { return m_string.find_first_not_of ( c,        startpos ); }
size_t xString::FindFirstNot(const xString& delims,     size_t startpos ) const { return m_string.find_first_not_of ( delims,   startpos ); }
size_t xString::FindLast    (char c,                    size_t offset   ) const { return m_string.find_last_of      ( c,        offset ); }
size_t xString::FindLast    (const xString& delims,     size_t offset   ) const { return m_string.find_last_of      ( delims,   offset ); }
size_t xString::FindLastNot (char c,                    size_t offset   ) const { return m_string.find_last_not_of  ( c,        offset ); }
size_t xString::FindLastNot (const xString& delims,     size_t offset   ) const { return m_string.find_last_not_of  ( delims,   offset ); }

xString& xString::RemoveAllMutable(char c)
{
    // removing chars can only shorten the string, which makes it an
    // ideal candidiate for in-place (mutable) string operations:

    auto orig_len = GetLength();
    int destidx = 0;
    for (int i=0; i<orig_len; ++i) {
        char ci = this->operator[](i);
        if (ci != c) {
            this->operator[](destidx++) = ci;
        }
    }

    Resize(destidx);
    return *this;
}

xString xString::RemoveAll(char c) const
{
    auto first = FindFirst(c, 0);
    if (first == npos) return *this;

    xString result;
    result.Resize(GetLength() - 1);

    int destidx = 0;
    for (int i=0; i<GetLength(); ++i) {
        char ci = this->operator[](i);
        if (ci != c) {
            result[destidx++] = ci;
        }
    }

    return result;
}

xString& xString::FormatV( const char* fmt, va_list list )
{
    // Formatting notes:
    // Once upon a time, formatting directly into std::string was frowned upon.
    // This was changed with the C++11 standard.
    //
    // (v)sprintf_s functions assert if the dest buffer isn't big enough to fit.
    // The correct usage is to calculate the length first using _vscprintf().

    int destSize = fmt ? _vscprintf( fmt, list ) : 0;
    bug_on_qa( destSize < 0, "Invalid string formatting parameters! -- or nasty old glibc?" );

    // Reserve truncates allocations to the specific size (why?), so don't use it.
    // Resize(), interestingly, does not resize allocs smaller, only larger as needed.
    //Reserve( (destSize + 511) & ~511 );

    // _vscprintf doesn't count terminating '\0', and resize() doesn't expect it either.
    // Thus, the following resize() will ensure +1 room for the null that vsprintf_s
    // will write.

    m_string.resize( destSize );
    if (destSize!=0)    // bypasses error that will occur on &m_string[0] (msvc runtime iterator checking)
        vsprintf_s( &m_string[0], destSize+1, fmt, list );

    return *this;
}

xString& xString::AppendFmtV( const char* fmt, va_list list )
{
    if (!fmt) return *this;

    size_t origlen  = GetLength();
    int destSize    = _vscprintf( fmt, list );

    bug_on_qa( destSize < 0, "Invalid string formatting parameters! -- or nasty old glibc?" );
    if (destSize==0) return *this;      // don't waste time appending nothing.

    m_string.resize( destSize+origlen );
    vsprintf_s( &m_string[origlen], destSize+1, fmt, list );

    return *this;
}

xString& xString::Format( const char* fmt, ... )
{
    va_list list;
    va_start( list, fmt );
    FormatV( fmt, list );
    va_end( list );

    return *this;
}

xString& xString::AppendFmt( const char* fmt, ... )
{
    va_list list;
    va_start( list, fmt );
    AppendFmtV( fmt, list );
    va_end( list );

    return *this;
}

#if TARGET_MSW
xString& xString::FormatV( const wchar_t* fmt, va_list list )
{
    if (!fmt) return *this;

    xstring_format_buffer<wchar_t> dest;
    _format_raw( dest, fmt, list );

    // have to convert back down to UTF8...
    Format( "%S", dest.ptr );

    return *this;
}

xString& xString::AppendFmtV( const wchar_t* fmt, va_list list )
{
    if (!fmt) return *this;

    xstring_format_buffer<wchar_t> dest;
    _format_raw( dest, fmt, list );

    // have to convert back down to UTF8...
    AppendFmt( "%S", dest.ptr );
    return *this;
}

xString& xString::Format( const wchar_t* fmt, ... )
{
    va_list list;
    va_start( list, fmt );
    FormatV( fmt, list );
    va_end( list );

    return *this;
}

xString& xString::AppendFmt( const wchar_t* fmt, ... )
{
    va_list list;
    va_start( list, fmt );
    AppendFmtV( fmt, list );
    va_end( list );

    return *this;
}
#endif

xString& xString::ToLowerMutable()
{
    std::transform(m_string.begin(), m_string.end(), m_string.begin(), ::tolower);
}

xString& xString::ToUpperMutable()
{
    std::transform(m_string.begin(), m_string.end(), m_string.begin(), ::toupper);
}

xString xString::ToLower() const
{
    xString result( *this );
    std::transform(result.m_string.begin(), result.m_string.end(), result.m_string.begin(), ::tolower);
    return result;
}

xString xString::ToUpper() const
{
    xString result( *this );
    std::transform(result.m_string.begin(), result.m_string.end(), result.m_string.begin(), ::toupper);
    return result;
}

// --------------------------------------------------------------------------------------
__eai bool xIsPathSeparator( const char& c )
{
#if TARGET_MSW || TARGET_ORBIS
    return (c == '/') || (c == '\\');
#else
    return (c == '/');
#endif
}

// --------------------------------------------------------------------------------------
bool xPathIsAbsolute( const xString& src )
{
    if (src.IsEmpty())                                  return false;
    if (xIsPathSeparator(src[0]))                       return true;

#if TARGET_MSW || TARGET_ORBIS
    // Some notes about absolute paths on Windows:
    //   * technically drives should be letters (alpha-only, non-numeric) but Windows/DOS
    //     parses files in such a way that "1:\" will always be considered as a drive specification.
    //      (one which cannot be fulfilled since the system cannot map drives to numbers!)
    //   * Drive letter without a backslash will use relative path on the drive, thus:
    //       a:folder\folder\file.txt -> a:\cwd_of_drive_a\folder\folder\file.txt

    if (src.GetLength() < 3)                            return false;
    if ((src[1] == ':') && xIsPathSeparator(src[2]))    return true;
#endif

    return false;
}

// --------------------------------------------------------------------------------------
xString xBaseFilename( const xString& src )
{
    static const char* PATH_SEP = TARGET_MSW ? "/\\" : "/";
    size_t pos = src.FindLast( PATH_SEP );
    if (pos == xString::npos) return src;
    return src.GetTail( pos+1 );
}

xString xBasePath( const xString& src )
{
    static const char* PATH_SEP = TARGET_MSW ? "/\\" : "/";
    size_t pos = src.FindLast( PATH_SEP );
    if (pos == xString::npos) return xString();     // no path separator?  means it's just a filename.  (could also return ./)
    return src.GetSubstring( 0, pos );
}

// --------------------------------------------------------------------------------------
xString xPath_Combine( const xString& left, const xString& right )
{
    if (right.IsEmpty())            return left;
    if (xPathIsAbsolute( right ))   return right;
    if (left.IsEmpty())             return right;

    const size_t lastpos = left.GetLength();
    assume (lastpos);       // should always be true since we escaped empty lefts above!
    if (!xIsPathSeparator(left[lastpos-1]))
    {
        #if TARGET_MSW
        return left + "\\" + right;
        #else
        return left + "/" + right;
        #endif
    }
    else
        return left + right;
}

xString xPath_Combine( const xString& left, const xString& right1, const xString& right2 )
{
    return xPath_Combine( xPath_Combine(left, right1), right2);
}

// --------------------------------------------------------------------------------------
//  xStringTokenizer  (implementations)
// --------------------------------------------------------------------------------------
bool xStringTokenizer::HasMoreTokens() const
{
    return ( m_string.FindFirstNot(m_delims, m_pos) != xString::npos );
}

xString xStringTokenizer::GetNextToken()
{
    xString token;
    do
    {
        if (!HasMoreTokens())
        {
            break;
        }

        size_t pos = m_string.FindFirst(m_delims, m_pos);

        if (pos == xString::npos)
        {
            token = m_string.GetTail(m_pos);
            m_pos = m_string.GetLength();
            m_lastDelim = 0;
        }
        else
        {
            size_t len = pos - m_pos;

            token = m_string.GetSubstring(m_pos, len);
            m_pos = pos + 1;

            m_lastDelim = m_string[pos];
        }
    }
    while (m_IgnoreEmpties && token.IsEmpty());

    return token;
}

// --------------------------------------------------------------------------------------
xString xPosixErrorStr( int errorval )
{
#if defined(_MSC_VER)
    char result[256];
    result[0] = 0;
    strerror_s( result, errorval );
#elif _GNU_SOURCE       // apparently turned on by SDL, or some other annoying dependency.
    char resultbuf[256];
    char* result = strerror_r( errorval, resultbuf, 255 );
#elif TARGET_ORBIS
    char result[256];
    result[0] = 0;
    strerror_s( result, sizeof(result), errorval );
#else
    // POSIX compliant version!
    char result[256];
    int code = strerror_r( errorval, result, 255 );
    if (code) result[0] = 0;
#endif
    if (errorval < 0)
        return xFmtStr( "[0x%08x] : %s", errorval, result[0] ? result : "strerror() itself has errored." );
    else
        return xFmtStr( "[%d] : %s",     errorval, result[0] ? result : "strerror() itself has errored." );
}

// --------------------------------------------------------------------------------------
xString xPosixErrorStr()
{
    return xPosixErrorStr(errno);
}

// --------------------------------------------------------------------------------------
//
// xPtrStr
//   use cPtrToStr() macro for passing parameters into printf/varadic functions.
//
__eai xString xPtrStr(const void* src, const char* sep)
{
#if TARGET_x32
    return xFmtStr( "0x%08X", src );
#else
    IntPack64 parts = IntPack64::fromPtr( src );
    return xFmtStr( "0x%08X%s%08X", parts.hi, sep ? sep : "", parts.lo );
#endif
}

__eai xString xPtrStr(VoidFunc* src, const char* sep)
{
    return xPtrStr( (void*)src, sep );
}

// ----------------------------------------------------------------------------
//  xHexStr (template implementations)
// ----------------------------------------------------------------------------

xString xHexStr( const u64& src, const char* sep )
{
    IntPack64 pack;
    pack = src;
    return xFmtStr( "0x%08x%s%08x", pack.hi, sep, pack.lo );
}

xString xHexStr( const s64& src, const char* sep )
{
    IntPack64 pack;
    pack = src;
    return xFmtStr( "0x%08x%s%08x", pack.hi, sep, pack.lo );
}

xString xHexStr( const u128& src, const char* sep )
{
    u128 pack = src;
    return xFmtStr( "0x%08x%s%08x%s%08x%s%08x", pack.w3, sep, pack.w2, sep, pack.w1, sep, pack.w0 );
}

//
// 8/16/32 bit varieties are provided for occasional template convenience (calling xHexStr from
// another templated function).  Otherwise, it is easier to use these via normal printf formatting.
//

xString xHexStr( const u8& src, const char* sep )
{
    return xFmtStr( "0x%02x", src );
}

xString xHexStr( const u16& src, const char* sep )
{
    return xFmtStr( "0x%04x", src );
}

xString xHexStr( const u32& src, const char* sep )
{
    return xFmtStr( "0x%08x", src );
}

xString xHexStr( const s32& src, const char* sep )
{
    return xFmtStr( "0x%08x", src );
}

// ----------------------------------------------------------------------------
//  xDecStr (template implementations)
// ----------------------------------------------------------------------------

xString xDecStr( const u64& src )
{
    return xFmtStr( FMT_U64, src );
}

xString xDecStr( const s64& src )
{
    return xFmtStr( FMT_S64, src );
}

#if !defined(_MSC_VER)
xString xDecStr( const long long& src )
{
    return xFmtStr( "%lld", src );
}
#endif

//
// 8/16/32 bit varieties are provided for occasional template convenience (calling xHexStr from
// another templated function).  Otherwise, it is easier to use these via normal printf formatting.
//

xString xDecStr( const u8& src )
{
    return xFmtStr( "%u", src );
}

xString xDecStr( const u16& src )
{
    return xFmtStr( "%u", src );
}

xString xDecStr( const u32& src )
{
    return xFmtStr( "%u", src );
}

xString xDecStr( const s8& src )
{
    return xFmtStr( "%d", src );
}

xString xDecStr( const s16& src )
{
    return xFmtStr( "%d", src );
}

xString xDecStr( const s32& src )
{
    return xFmtStr( "%d", src );
}


// ----------------------------------------------------------------------------
//  xDataStr (implementations)
// ----------------------------------------------------------------------------
// Data is printed low-to-high, where-as xHexStr prints 64/128 bit stuff high-to-low.
// Also, data always assumes a space separator (ideal for hex views) and no 0x prefies.
//
xString xDataStr( const u64& src )
{
    IntPack64 pack;
    pack = src;
    return xFmtStr( "%08x %08x", pack.lo, pack.hi );
}

xString xDataStr( const s64& src )
{
    IntPack64 pack;
    pack = src;
    return xFmtStr( "%08x %08x", pack.lo, pack.hi );
}

xString xDataStr( const u128& pack )
{
    return xFmtStr( "%08x %08x %08x %08x", pack.w0, pack.w1, pack.w2, pack.w3 );
}

xString xDataStr( const __m128& src )
{
    u128 pack = u128::fromQuad(src);
    return xFmtStr( "%08x %08x %08x %08x", pack.w0, pack.w1, pack.w2, pack.w3 );
}

xString xDataStr( const u32& src )
{
    return xFmtStr( "%08x", src );
}

// ----------------------------------------------------------------------------
xString xDataAsciiStr( const void* src, size_t size )
{
    xString result;
    result.Resize(size);

    const   u8*     src8 = (u8*)src;
            char*   dst8 = &result[0];      // cheats! but C++11 promises this works now. :)

    for( size_t i=0; i<size; ++i )
    {
        dst8[i] = '?';
        if (src8[i] >= 32 && src8[i] <= 126)
            dst8[i] = src8[i];
    }
    return result;
}
