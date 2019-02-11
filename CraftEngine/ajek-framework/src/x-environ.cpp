
#include "PCH-framework.h"
#include <cstdlib>

#if TARGET_MSW
// Microsoft never implemented setenv...
int setenv(const char* var, const char* value, int overwrite)
{
    errno = 0;

    if (!var) {
        errno = EINVAL;
        return -1;
    }

    const auto* exists = getenv(var);
    if (!exists) return 0;

    const auto* equalsCheck = var;
    while(equalsCheck) {
        if (equalsCheck[0] == '=') {
            errno = EINVAL;
            return -1;
        }
    }

    int err = _putenv_s(var, value);
    if (err) return -1;
}
#endif

bool xEnvironExists(const xString& varname)
{
    return getenv(varname) != nullptr;
}

// Returns empty string if the environment var does not exist.
// Use xEnvironExists() if you need to differentiate between empty and unset environ vars
xString xEnvironGet(const xString& varname)
{
    size_t reqlen;
    xString result;
    int err;

    err = getenv_s(&reqlen, NULL, 0, varname);
    x_abort_on(err, "getenv_s(%s) failed with code %d(%s)", varname.c_str(), err, cPosixErrorStr(err));
    if (!reqlen) { return xString(); }

    result.Resize(reqlen);
    err = getenv_s(&reqlen, result.data(), reqlen, varname);
    x_abort_on(err, "getenv_s(%s) failed with code %d(%s)", varname.c_str(), err, cPosixErrorStr(err));
    return result;
}

void xEnvironSet(const xString& varname, const xString& value, bool overwrite)
{
    int err = setenv(varname, value, overwrite);
    x_abort_on(err, "setenv(%s, %s) failed with code %d(%s)", varname.c_str(), value.c_str(), err, cPosixErrorStr(err));
}
