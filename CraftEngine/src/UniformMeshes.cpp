
#include "PCH-rpgcraft.h"

#include "UniformMeshes.h"

UniformMeshVertex g_mesh_UniformQuad[4] =
{
	{ vFloat2(0.0f,  0.0f), vFloat2(0.0f,  0.0f) },
	{ vFloat2(0.0f,  1.0f), vFloat2(0.0f,  1.0f) },
	{ vFloat2(1.0f,  1.0f), vFloat2(1.0f,  1.0f) },
	{ vFloat2(1.0f,  0.0f), vFloat2(1.0f,  0.0f) },
};

UniformMeshVertex g_mesh_UniformQuadHalf[4] =
{
	{ vFloat2(0.0f,  0.0f), vFloat2(0.0f,  0.0f) },
	{ vFloat2(0.0f,  0.5f), vFloat2(0.0f,  1.0f) },
	{ vFloat2(0.5f,  0.5f), vFloat2(1.0f,  1.0f) },
	{ vFloat2(0.5f,  0.0f), vFloat2(1.0f,  0.0f) },
};

s16 g_ind_UniformQuad[6] = {
	0,1,3,
	1,3,2
};
