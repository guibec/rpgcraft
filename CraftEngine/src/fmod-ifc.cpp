#include "PCH-rpgcraft.h"

#include "x-assertion.h"
#include "fmod.hpp"

#pragma comment(lib,"fmod64_vc")

FMOD::System*		fmsys;

void fmod_CheckLib()
{
	FMOD_RESULT		result;
	unsigned int    version;

	/*
		Create a System object and initialize.
	*/
	result = FMOD::System_Create(&fmsys);
	bug_on(result);

	result = fmsys->getVersion(&version);
	bug_on(result);

	if (version < FMOD_VERSION)
	{
		log_and_abort("FMOD lib version %08x doesn't match header version %08x", version, FMOD_VERSION);
	}
}


void fmod_InitSystem()
{
	FMOD_RESULT		result;
	void           *extradriverdata = 0;

	result = fmsys->init(32, FMOD_INIT_NORMAL, extradriverdata);
	bug_on(result);
}
