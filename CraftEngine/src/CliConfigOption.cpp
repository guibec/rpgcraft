#include "PCH-rpgcraft.h"

#include "x-types.h"
#include "x-stdfile.h"
#include "ajek-script.h"
#include "appConfig.h"

#include "x-stl.h"
#include <type_traits>

#if !defined(VALIDATE_CLI_OPTIONS_LIST)
#   define VALIDATE_CLI_OPTIONS_LIST     1
#endif

static std::vector<xString> s_asset_search_dirs;
static bool s_release_check_mode = false;

enum CliResultType
{
    CLI_OK,
    CLI_PARSE_WARNING,      // warnings turn into errors when release_check = 1
    CLI_PARSE_ERROR,
};

struct CliParseState {
    xString         option;
    xString         value;

    std::function<xString()>  diag_filepos;       // filename and line, pre-formatted for diagnostic printout

    void        log_problem(const char* fmt, ...);
    bool        boundscheck_int(int input, int lower, int upper);
};

typedef void Fn_CliParseCb(const xString& value);

struct CliOptionDesc {
    const char*     name;
    Fn_CliParseCb*  func;
};


static inline bool _sopt_isWhitespace( char ch )
{
    return !ch
        || (ch == '\r')
        || (ch == '\n')
        || (ch == ' ')
        || (ch == '\t');
}

static inline bool _sopt_is_number_flag( char ch )
{
    auto chl = tolower(ch);
    return (chl == 'f') || (chl == 'd');
}


bool CliParseState::boundscheck_int(int input, int lower, int upper)
{
    if (input >= lower && input < upper) {
        return true;
    }

    log_problem("Integer must be in the range of %d to %d", lower, upper);
    return false;
}

void CliParseState::log_problem(const char* fmt, ...)
{
    va_list arglist;
    va_start(arglist,fmt);
    auto formatted = xString().FormatV(fmt, arglist);
    va_end(arglist);

    if (s_release_check_mode) {
        x_abort("%s%s", diag_filepos().c_str(), formatted.c_str());
    }
    else {
        warn_host("%s%s", diag_filepos().c_str(), formatted.c_str());
    }
}

static thread_local CliParseState* s_cli;

// when endpos=nullptr, the function expects the number to end with either \0 or whitespace.
// Any non-whitespace character is considered an error.
bool to_s64(s64& dest, const char* src, char** endpos=nullptr, int radix=0)
{
    // strtol / strtoll / strtod have some strange behavior about handling overflow.
    // It supposedly returns LLONG_MAX and sets errno=ERANGE, and sets endpos to the last character at
    // the end of the number (not the last character before overflow occurred).
    // For our purposes, it's OK to just force errno=0 before calls and not bother with this LLONG_MAX mess.
    //
    // Most use cases should do their own bounds checking anyway.  Almost nothing _actually_ supports full-
    // range 64-bit ints.

    char* local_endpos = nullptr;
    if (!endpos) {
        endpos = &local_endpos;
    }

    errno = 0;
    auto result = strtoll(src, endpos, radix);
    if (errno) {
        s_cli->log_problem("toInt64(src='%s') failed: %s", src, strerror(errno));
        return false;
    }
    if (local_endpos && (local_endpos[0] == '.')) {
        errno = 0;
        auto result_f = strtof(src, endpos);
        if (errno) {
            // as far as I know, this shouldn't really happen (?)
            s_cli->log_problem("toInt64(src='%s') strtof failed: %s", src, strerror(errno));
        }
        elif (result != (s64)result_f) {
            s_cli->log_problem("toInt64(src='%s') floating point value will be truncated", src, result);
        }
        if (local_endpos && _sopt_is_number_flag(local_endpos[0])) {
            local_endpos += local_endpos[0] ? 1 : 0;
        }
    }

    if (local_endpos && (!_sopt_isWhitespace(local_endpos[0]))) {
        s_cli->log_problem("toInt64(src='%s') unexpected char(s) after integer", src, result);
    }

    dest = result;
    return true;
}

// when endpos=nullptr, the function expects the number to end with either \0 or whitespace.
// Any non-whitespace character is considered an error.
bool to_double(double& dest, const char* src, char** endpos=nullptr)
{
    char* local_endpos = nullptr;
    if (!endpos) {
        endpos = &local_endpos;
    }

    auto result = strtof(src, endpos);
    if (errno) {
        s_cli->log_problem("to_double(src='%s') failed: %s", src, strerror(errno));
    }

    if (local_endpos && (!_sopt_isWhitespace(local_endpos[0] && !_sopt_is_number_flag(local_endpos[0])))) {
        s_cli->log_problem("toInt64(src='%s') unexpected char(s) after number", src, result);
    }

    dest = result;
    return true;
}

template<typename T>
bool strtoanyint(T& dest, const char* src, char** endpos=nullptr, int radix=0)
{
    // this implementation works on the restriction that full-range u64 (unsigned 64-bit) is
    // not supported.  Given that restriction, full range unsigned 32, 16, and 8 bit values can
    // be supported since their unsigned representations readily fit within the signed-64 bit space.

    s64 full_result;
    bool success = to_s64(full_result, src, endpos, radix);
    if (success) {
        if (full_result != (T)full_result) {
            // log out ranges ony for word/byte size types.
            s_cli->log_problem("strtoanyint<%c%d>(src='%s', radix=%d) integer overflow. %s",
                std::is_unsigned<T>::value ? 'u' : 's', sizeof(T) * 8, src, radix,
                ((sizeof(T) <= 2)
                    ? cFmtStr("Valid range is %d to %d.", std::numeric_limits<T>::min(), std::numeric_limits<T>::max())
                    : ""
                )
            );
            success = 0;
        }
    }
    if (success) {
        dest = (T)full_result;
    }
    return success;
}

template<typename T>
bool strtoanynum(T& dest, const char* src, char** endpos=nullptr, int radix=0)
{
    // this implementation works on the restriction that full-range u64 (unsigned 64-bit) is
    // not supported.  Given that restriction, full range unsigned 32, 16, and 8 bit values can
    // be supported since their unsigned representations readily fit within the signed-64 bit space.

    s64 full_result;
    bool success = to_s64(full_result, src, endpos, radix);
    if (success) {
        if (full_result != (T)full_result) {
            // log out ranges ony for word/byte size types.
            s_cli->log_problem("strtoanyint<%c%d>(src='%s', radix=%d) integer overflow. %s",
                std::is_unsigned<T>::value ? 'u' : 's', sizeof(T) * 8, src, radix,
                ((sizeof(T) <= 2)
                    ? cFmtStr("Valid range is %d to %d.", std::numeric_limits<T>::min(), std::numeric_limits<T>::max())
                    : ""
                )
            );
            success = 0;
        }
    }
    if (success) {
        dest = (T)full_result;
    }
    return success;
}


bool to_int2(int2& dest, const xString& src)
{
    xStringTokenizer tok(",", src);
    if (!strtoanyint(dest.x, tok.GetNextToken())) return false;
    if (!strtoanyint(dest.y, tok.GetNextToken())) return false;
    if (tok.HasMoreTokens()) {
        s_cli->log_problem("%signoring extra tokens in value");
    }
    return true;
}

bool to_bool(bool& dest, const xString& value)
{
    auto lower = value.ToLower();
    if (value == "0" || lower == "false" || lower == "off" || lower == "no" ) { dest = false; return dest; }
    if (value == "1" || lower == "true"  || lower == "on"  || lower == "yes") { dest = true;  return dest; }

    s_cli->log_problem("expected boolean r-value [0,1,yes,no,true,false,on,off]");
    return dest;
}

// all options are listed once here.
// Options are referenced again, by string name, when parsed.
static const CliOptionDesc s_valid_options_list[] = {
    { "release-check"               ,[](const xString& value){
        to_bool(s_release_check_mode, value);
    }},
    { "asset-search-dir"            ,[](const xString& value){
        if (value == "/clear/") {
            s_asset_search_dirs.clear();
        }
        else {
            s_asset_search_dirs.push_back(value);
        }
    }},
    { "load-cli"                    ,[](const xString& value){
        if (xFileStat(value).IsFile()) {
            CliParseFromFile(value);
        }
        else {
            s_cli->log_problem("cannot stat cli file: %s", value.c_str());
        }
    }},
    { "window-client-pos"           ,[](const xString& value){
        g_settings_hostwnd.has_client_pos  |= to_int2(g_settings_hostwnd.client_pos,  value);
    }},
    { "window-client-size"          ,[](const xString& value){
        g_settings_hostwnd.has_client_size |= to_int2(g_settings_hostwnd.client_size, value);
    }},

    // Visual Studio script debug mode:
    // Script error messages should be printed relative to the solution directory.
    { "script-dbg-relpath"          ,[](const xString& value){
        AjekScript_SetDebugRelativePath(value);
    }},
};

bool IsReleaseCheckMode()
{
    return s_release_check_mode;
}

// validate option against known list.
// Returns result of the search.  True for _exact_ matches only.
// Logs to console if nearest match suggesions were found.
Fn_CliParseCb* validate_option_string(const xString& option)
{
    for(const auto& item : s_valid_options_list) {
        if (option == item.name) {
            return item.func;
        }
    }

    // no exact matches.  Begin weighted nearest-match search now.
    // (oh noes, not implemented !!)

    s_cli->log_problem("unrecognized option '%s'", option.c_str());
    return nullptr;
}


void CliParseOption(const xString& utf8)
{
    CliParseOptionRaw(utf8,
        [utf8]() {
            return xFmtStr("error parsing cli arg '%s': ", utf8.c_str());
        },
        utf8.StartsWith("--") ? 2 : 0          // readpos = 2, skips '--'
    );
}

void CliParseOptionRaw(const xString& utf8, const std::function<xString()>& file_lineno_diag, int startpos)
{
    // Tokenizer.
    //   * only care about double dash "--" anything lacking a double-dash prefix is an error.
    //   * all switches require assignment operator '=', eg:
    //        --option=value
    //   * Boolean toggles are required to specify --arg=0 or --arg=1
    //   * whitespace in options is disallowed
    //   * Whitespace is handled by the shell command line processor, so any whitespace is assumed part of the value.
    //   * environment variable expansion not supported (expected to be performed by shell)
    //
    // Windows-sepcific notes:
    //   * __argv has double-quotes already processed, but unfortunately Windows has some rather dodgy quotes parsing,
    //     so it's likely there'll be spurious quotes lying around when injecting Visual Studio EnvVars.  For now it's
    //     responsibility of user to fix those.
    //   * __argv does not process single quotes. These will be present and will result in malformed CLI syntax.
    //

    char optmp[128];
    const char* src = utf8.data();

    int readpos  = startpos;
    int writepos = 0;
    for(;src[readpos];) {
        x_abort_on(_sopt_isWhitespace(utf8.data()[readpos]), "%swhitespace is not allowed in option", file_lineno_diag().c_str());
        x_abort_on(writepos >= bulkof(optmp)-1,              "%soption exceeds length limit of %d"  , file_lineno_diag().c_str(), bulkof(optmp)-1);

        optmp[writepos] = utf8.data()[readpos++];
        if (optmp[writepos] == '=') break;
        ++writepos;
    }

    x_abort_on(optmp[writepos] != '=', "%sassignment operator '=' is required", file_lineno_diag().c_str());
    x_abort_on(!writepos,              "%szero-length option not allowed"     , file_lineno_diag().c_str());

    optmp[writepos] = 0;

    if (!s_cli) {
        s_cli = new CliParseState;
    }

    if (auto* fnptr = validate_option_string(optmp)) {
        // setup thread local storage and then invoke the lambda for this option.
        // (thread local storage is only used for diagnostic purposes)
        s_cli->value            = utf8.data() + readpos;
        s_cli->option           = optmp;
        s_cli->diag_filepos     = file_lineno_diag;
        fnptr(s_cli->value);
    }
}

template<typename T>
xString toStringIndented(const T& container)
{
    xString result;

    for(const auto& path : container) {
        if (path.IsEmpty()) continue;
        if (!result.IsEmpty()) {
            result += "\n    ";
        }
        result += path;
    }
    return result;
}

// TryFindAsset, for situations where a non-standard error is needed.
xString TryFindAsset(const xString& src)
{
    bug_on (!xPathIsUniversal(src), "path layout is not universal: %s", src.c_str());

    if (src.StartsWith('/')) {
        // absolute (rooted) path, no point in trying it against search paths
        return src;
    }
    for(const auto& path : s_asset_search_dirs) {
        auto fullpath = path + "/" + src;
        if (xFileStat(fullpath).Exists()) {
            return fullpath;
        }
    }
    return xString();
}

// FindAsset - searches specified asset paths for the goods.
xString FindAsset(const xString& src)
{
    auto result = TryFindAsset(src);
    x_abort_on(result.IsEmpty(), "Could not find asset: %s\nSearch dirs:\n", src.c_str(), toStringIndented(s_asset_search_dirs).c_str());
    return result;
}


// accepts a delimited list of input paths.  Meant for reading a list of pathnames from an environment
// variable via getenv().  Alternatives that don't depend on a delimiter should be used otherwise.
//
// Delimiter is semicolon on windows and colon for the rest fo the world.  The delimiter is systematically
// replaced with NUL, which allows it to be parsed by a universal cross-platform parser.
//
xString EnvPathList_Normalize(const xString& src_list)
{
    const char delimiter = TARGET_MSW ? ';' : ':';

    xString result;
    result.Resize(src_list.GetLength() + 1);
    const char* src = src_list.data();
          char* dst = result.data();

    // walk the string and replace delimiter with NUL.  Result string is always same or shorter
    // length than original.

    int didx = 0;
    for(;src[0]; ++src) {
        dst[didx] = src[0];
        if (dst[didx] == '\\' && src[1] == delimiter) {     // backslash to escape delimiter
            continue;
        }
        if (dst[didx] == delimiter) {
            dst[didx] = 0;
        }
        ++didx;
    }

    bug_on(didx >= result.GetLength());
    dst[didx] = 0;
    result.Resize(didx);

}

// -------------------------------------------------------------------------------------
//  Fun Side Project: Nearness matching of cli options!!
// -------------------------------------------------------------------------------------
// There's a nearness matcher called levenshtein, which is really simple, super popular,
// and also sucks to the point that you're more likely to improve user experience by _not_
// using it.  About the only thing it handles well is typos where a character has been
// omitted.  Damerau–Levenshtein handles omissions and juxtapositions, making it slightly
// less useless but still pretty damn useless for most actual programmer-style typos.
//
// what we actually care about when matching strings is:
//  1. physical nearness to other characters on the physical keyboard
//        LiteRail -> KiteFail
//  2. upper/lowercase mismatches
//  3. juxtaposing entire sub-words, for example:
//        StrongTalk -> TalkStrong
//     (in the case of cli options, juxtapositions should occur along hyphen delimiters, mostly)
//
// distance: sqrt( (x2-x1)^2 + (y2-y1)^2 );
//

const char* const qwerty[4] = {
    "1234567890-=",
    "qwertyuiop[]",
    "asdfghjkl;' ",
    "zxcvbnm,./  ",
};

const char* const dvorak[4] = {
    "1234567890[]",
    "',.pyfgcrl/=",
    "aoeuidhtns- ",
    ";qjkxbmwvz  ",
};

// For distance matching we only really care about alphanumerics.
float2 s_qwerty_coordinate_map[0x30];

void InitDistanceMap(float2 (&dest)[0x30], const char* const (&keyboard_map)[4]) {
    for(int taby=0; taby<4; ++ taby) {
        float ypos = float(taby);
        float xpos = (ypos * 0.33f);
        const char* row = keyboard_map[taby];
        for(int tabx=0; row[tabx]; ++tabx, xpos += 1.0f) {

            // don't care to map/index chars that don't fit in our table.
            int coord_idx = toupper(row[tabx]) - 0x30;
            if (coord_idx < bulkof(dest)) {
                dest[coord_idx] = { ypos, xpos };
            }
        }
    }
}

// WIP - from here the plan would be to delimit the input by hyphen and search for juxtapositions
// A more robust approach would ignore hyphens and do heuristic search of juxtapositions, but that
// sounds like work and grounds for weird false positives if not implemented well, so let's not
// go there.
