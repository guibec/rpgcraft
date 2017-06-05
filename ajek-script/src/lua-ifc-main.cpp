
#include "x-types.h"
#include "x-stl.h"
#include "x-stdlib.h"
#include "x-string.h"

#include "x-assertion.h"

#include "ajek-script.h"
#include "lua-ifc-internal.h"

extern "C" {
#	include "linit.c"
}

DECLARE_MODULE_NAME("lua-main");


// Goals:
//  1. create module for engine parameter setup.   Debug settings, running environment, paths, so on.
//  2. create module for specifying behavior of tile engine things.
//       tilesets to load.
//       starting position.
//       response to user input.