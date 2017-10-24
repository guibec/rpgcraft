#pragma once

#include "v-float.h"
#include "x-gpu-ifc.h"

struct UniformMeshVertex
{
	vFloat2		xy;
	vFloat2		uv;
};

extern UniformMeshVertex	g_mesh_UniformQuad		[4];
extern UniformMeshVertex	g_mesh_UniformQuadHalf	[4];
extern s16					g_ind_UniformQuad		[6];

extern GPU_IndexBuffer		g_idx_box2D;

extern void UniformMeshes_InitGlobalResources();

#include <DirectXMath.h>

using DirectX::XMMATRIX;
using DirectX::XMVectorSet;
using DirectX::XMMatrixLookAtLH;
using DirectX::XMMatrixOrthographicLH;

struct GPU_ViewCameraConsts
{
	XMMATRIX View;
	XMMATRIX Projection;
};

