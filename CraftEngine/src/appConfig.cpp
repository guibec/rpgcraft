
#include "PCH-rpgcraft.h"

#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"

#include "appConfig.h"
#include "ajek-script.h"


DECLARE_MODULE_NAME("config");

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

        const char* src = curline.data();
              char* dst = curline.data();

        char quotes_mode = 0;

        for (; src[0]; ++src) {
            if (src[0] == quotes_mode) {
                quotes_mode = 0;
                continue;
            }
            if (!quotes_mode && (src[0] == '"') || (src[0] == '\'')) {
                quotes_mode = src[0];
                continue;
            }

            dst[0] = src[0];
            ++dst;
        }
        auto newsize = dst - curline.data();
        bug_on(dst > src);
        curline.Resize(dst - curline.data());

        if (quotes_mode) {
            x_abort("%s(%d): error unclosed %s-quote", srcfullpath.c_str(), lineno, quotes_mode == '"' ? "double" : "single");
        }

        CliParseOptionRaw(curline, xFmtStr("%s(%d): ", srcfullpath.c_str(), lineno), 2);
    }
}
