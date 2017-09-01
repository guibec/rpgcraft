#pragma once

#include "v-float.h"

struct UniformMeshVertex
{
	vFloat2		xy;
	vFloat2		uv;
};

extern UniformMeshVertex	g_mesh_UniformQuad	[4];
extern s16					g_ind_UniformQuad	[6];