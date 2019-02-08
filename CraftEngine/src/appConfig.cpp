
#include "PCH-rpgcraft.h"

#include "x-types.h"
#include "x-stdfile.h"
#include "x-stl.h"
#include "x-assertion.h"

#include "appConfig.h"
#include "ajek-script.h"


HostwindowSettings g_settings_hostwnd;


void CliParseFromFile(const xString& srcfullpath)
{
    auto* fp = xFopen(srcfullpath, "rb");
    x_abort_on(!fp, "failed to open file: %s", srcfullpath.c_str());
    Defer(if (fp) { fclose(fp); });

    // cli.txt files are sort of a super-lightweight line-delimited syntax.
    //  - leading and trailing whitespace (' ' and tab) are ignored (use quotes to
    //    pass trailing whitespace into the command line).
    //  - if the first non-whitespace character is '#' or ';', the line is ignored (comment)

    xString curline;
    bool    eof = 0;
    int     lineno = 0;

    auto getdiagstr_file_lineno = [&]() {
        return xFmtStr("error parsing cli:\n%s(%d): ", Host_GetFullPathName(srcfullpath).c_str(), lineno);
    };

    while (!eof) {
clear_line:
        curline.Clear();

append_line:
        eof = !xFgets(curline, fp);
        ++lineno;
        if (curline.IsEmpty()) {
            if (eof) break;
            goto append_line;
        }

        if (curline.EndsWith('\\')) {
            // strip the backslash and append the next line...
            // but only if it doesn't end in double-backslash!
            curline.Resize(curline.GetLength()-1);
            if (!curline.EndsWith('\\')) {
                curline += '\n';
                goto append_line;
            }
        }

        auto pos = curline.FindFirstNot(" \t", 0);
        if (pos == xString::npos) {
            goto clear_line;
        }

        if (curline[pos] == '#' || curline[pos] == ';') {
            // comments are any line that begin with '#' or ';'
            goto clear_line;
        }

        // In theory this format is so simple that we could pass quotes as-is into the
        // CliParseOptions.  The caveat is human expectations and existing tool behaviors:
        // Windows platforms in particular love to auto-wrap filenames in quotes, and humans
        // in general expect to have to wrap spaces in quotes.  Finally, if we want to pass
        // trailing spaces into an option, it's a good idea to do so using quotes, again because
        // neither human nor most editors/tools expect trailing whitespace to be significant.
        //  (or put the other way, they all expect it to be stripped).
        //
        // Therefore we provide rudimentary quotes parsing here.
        // To further match user behavior expectations in files, quotes are only allowed on rvalues.

        const char* src = curline.data();
              char* dst = curline.data();

        char quotes_mode = 0;
        bool is_rvalue = 0;
        bool is_leading_white  = 1;
        bool is_trailing_white = 0;

        for (; src[0]; ++src) {
            if (is_leading_white && src[0] == ' ') continue;
            is_leading_white = 0;

            bool is_any_quote = (src[0] == '"') || (src[0] == '\'');
            bool is_any_space = (src[0] == ' ') || (src[0] == '\t');

            x_abort_on(!is_rvalue && is_any_quote, "%squotes are not allowed in l-values", getdiagstr_file_lineno().c_str());

            if (is_rvalue) {
                if (src[0] == quotes_mode) {
                    quotes_mode = 0;
                    continue;
                }
                if (!quotes_mode && is_any_quote) {
                    quotes_mode = src[0];
                    continue;
                }
            }
            else {
                if (src[0] == '=') {
                    is_rvalue = 1;
                    is_leading_white  = 1;
                    is_trailing_white = 0;
                }
                elif (is_trailing_white) {
                    if (is_any_space) continue;

                    x_abort("%sl-values cannot contain whitespace", getdiagstr_file_lineno().c_str());
                }
                elif (is_any_space) {
                    is_trailing_white = 1;
                    continue;
                }
            }

            dst[0] = src[0];
            ++dst;
        }
        auto newsize = dst - curline.data();
        bug_on(dst > src);
        curline.Resize(dst - curline.data());

        // remove trailing whitespace .. just tabs and spaces.  Newlines had to have been escaped
        // and so those should be honored as part of the entire string.

        auto trailing_space_pos = curline.FindLastNot(" \t");
        if (trailing_space_pos != xString::npos) {
            curline.Erase(trailing_space_pos+1);
        }

        if (quotes_mode) {
            x_abort("%serror unclosed %s-quote", getdiagstr_file_lineno().c_str(), quotes_mode == '"' ? "double" : "single");
        }

        CliParseOptionRaw(curline, getdiagstr_file_lineno);
    }
}
