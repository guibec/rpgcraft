#pragma once

#include "v-float.h"

struct UniformMeshVertex
{
	vFloat2		xy;
	vFloat2		uv;
};

extern UniformMeshVertex	g_mesh_UniformQuad		[4];
extern UniformMeshVertex	g_mesh_UniformQuadHalf	[4];
extern s16					g_ind_UniformQuad		[6];

#include <DirectXMath.h>

using DirectX::XMMATRIX;
using DirectX::XMVectorSet;
using DirectX::XMMatrixLookAtLH;

struct GPU_ViewCameraConsts
{
	XMMATRIX View;
	XMMATRIX Projection;
};

extern XMMATRIX         g_Projection;

