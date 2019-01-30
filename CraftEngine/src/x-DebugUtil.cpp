
#include "PCH-rpgcraft.h"

#include "x-types.h"
#include "x-string.h"
#include "x-stdfile.h"


xString xGetTempDir()
{
    xString temp_dir = xEnvironGet("RPGCRAFT_TEMP_DIR");
    if (temp_dir.IsEmpty()) {
        // read temp dir from global settings
    }

    if (temp_dir.IsEmpty()) {
        temp_dir = "../dbgtmp";
    }
    xCreateDirectory(temp_dir);
    return temp_dir;
}
