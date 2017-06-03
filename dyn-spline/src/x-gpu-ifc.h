#pragma once

#include "x-types.h"
#include "x-simd.h"

enum GPU_VertexBufferLayoutType {
	VertexBufferLayout_Color,
	VertexBufferLayout_Tex1,
	VertexBufferLayout_ColorTex1,
	VertexBufferLayout_ColorTex4,
};

enum GPU_ResourceFmt
{
	GPU_ResourceFmt_R32G32B32A32_TYPELESS       = 1,
	GPU_ResourceFmt_R32G32B32A32_FLOAT          = 2,
	GPU_ResourceFmt_R32G32B32A32_UINT           = 3,
	GPU_ResourceFmt_R32G32B32A32_SINT           = 4,
	GPU_ResourceFmt_R32G32B32_TYPELESS          = 5,
	GPU_ResourceFmt_R32G32B32_FLOAT             = 6,
	GPU_ResourceFmt_R32G32B32_UINT              = 7,
	GPU_ResourceFmt_R32G32B32_SINT              = 8,
	GPU_ResourceFmt_R16G16B16A16_TYPELESS       = 9,
	GPU_ResourceFmt_R16G16B16A16_FLOAT          = 10,
	GPU_ResourceFmt_R16G16B16A16_UNORM          = 11,
	GPU_ResourceFmt_R16G16B16A16_UINT           = 12,
	GPU_ResourceFmt_R16G16B16A16_SNORM          = 13,
	GPU_ResourceFmt_R16G16B16A16_SINT           = 14,
	GPU_ResourceFmt_R32G32_TYPELESS             = 15,
	GPU_ResourceFmt_R32G32_FLOAT                = 16,
	GPU_ResourceFmt_R32G32_UINT                 = 17,
	GPU_ResourceFmt_R32G32_SINT                 = 18,
	GPU_ResourceFmt_R32G8X24_TYPELESS           = 19,
	GPU_ResourceFmt_D32_FLOAT_S8X24_UINT        = 20,
	GPU_ResourceFmt_R32_FLOAT_X8X24_TYPELESS    = 21,
	GPU_ResourceFmt_X32_TYPELESS_G8X24_UINT     = 22,
	GPU_ResourceFmt_R10G10B10A2_TYPELESS        = 23,
	GPU_ResourceFmt_R10G10B10A2_UNORM           = 24,
	GPU_ResourceFmt_R10G10B10A2_UINT            = 25,
	GPU_ResourceFmt_R8G8B8A8_TYPELESS           = 27,
	GPU_ResourceFmt_R8G8B8A8_UNORM              = 28,
	GPU_ResourceFmt_R8G8B8A8_UINT               = 30,
	GPU_ResourceFmt_R8G8B8A8_SNORM              = 31,
	GPU_ResourceFmt_R8G8B8A8_SINT               = 32,
	GPU_ResourceFmt_R16G16_TYPELESS             = 33,
	GPU_ResourceFmt_R16G16_FLOAT                = 34,
	GPU_ResourceFmt_R16G16_UNORM                = 35,
	GPU_ResourceFmt_R16G16_UINT                 = 36,
	GPU_ResourceFmt_R16G16_SNORM                = 37,
	GPU_ResourceFmt_R16G16_SINT                 = 38,
	GPU_ResourceFmt_R32_TYPELESS                = 39,
	GPU_ResourceFmt_D32_FLOAT                   = 40,
	GPU_ResourceFmt_R32_FLOAT                   = 41,
	GPU_ResourceFmt_R32_UINT                    = 42,
	GPU_ResourceFmt_R32_SINT                    = 43,
	GPU_ResourceFmt_R24G8_TYPELESS              = 44,
	GPU_ResourceFmt_D24_UNORM_S8_UINT           = 45,
	GPU_ResourceFmt_R24_UNORM_X8_TYPELESS       = 46,
	GPU_ResourceFmt_X24_TYPELESS_G8_UINT        = 47,
	GPU_ResourceFmt_R8G8_TYPELESS               = 48,
	GPU_ResourceFmt_R8G8_UNORM                  = 49,
	GPU_ResourceFmt_R8G8_UINT                   = 50,
	GPU_ResourceFmt_R8G8_SNORM                  = 51,
	GPU_ResourceFmt_R8G8_SINT                   = 52,
	GPU_ResourceFmt_R16_TYPELESS                = 53,
	GPU_ResourceFmt_R16_FLOAT                   = 54,
	GPU_ResourceFmt_D16_UNORM                   = 55,
	GPU_ResourceFmt_R16_UNORM                   = 56,
	GPU_ResourceFmt_R16_UINT                    = 57,
	GPU_ResourceFmt_R16_SNORM                   = 58,
	GPU_ResourceFmt_R16_SINT                    = 59,
	GPU_ResourceFmt_R8_TYPELESS                 = 60,
	GPU_ResourceFmt_R8_UNORM                    = 61,
	GPU_ResourceFmt_R8_UINT                     = 62,
	GPU_ResourceFmt_R8_SNORM                    = 63,
	GPU_ResourceFmt_R8_SINT                     = 64,
	GPU_ResourceFmt_A8_UNORM                    = 65,
};

enum GpuPrimitiveType
{
	GPU_PRIM_POINTLIST		= 1,
	GPU_PRIM_LINELIST		= 2,
	GPU_PRIM_LINESTRIP		= 3,
	GPU_PRIM_TRIANGLELIST	= 4,
	GPU_PRIM_TRIANGLESTRIP	= 5,

	// DX11 doesn't support TriFan natively, but PS4 HW does.
	// Maybe TriFan topology can be set and then submitted indexes converted automatically to suit DX11 limitations?
};

enum GpuRasterFillMode {
	GPU_Fill_Solid		= 0,
	GPU_Fill_Wireframe	= 1,
	_GPU_Fill_Count_,
};

enum GpuRasterCullMode {
	GPU_Cull_None			= 0,
	GPU_Cull_Front			= 1,
	GPU_Cull_Back			= 2,
	_GPU_Cull_Count_
};

enum GpuRasterScissorMode {
	GPU_Scissor_Disable		= 0,
	GPU_Scissor_Enable		= 1,
	_GPU_Scissor_Count_
};

struct GPU_VertexBuffer {
	sptr		m_driverData;		// can be either memory pointer or handle index into table (driver-dependent)
	GPU_VertexBuffer(const void* driverData = nullptr);
};

struct GPU_IndexBuffer {
	sptr		m_driverData;		// can be either memory pointer or handle index into table (driver-dependent)
	GPU_IndexBuffer(const void* driverData = nullptr);
};

struct GPU_ShaderVS {
	sptr		m_driverData;		// can be either memory pointer or handle index into table (driver-dependent)
	GPU_ShaderVS(const void* driverData = nullptr);
};

struct GPU_ShaderFS {
	sptr		m_driverData;		// can be either memory pointer or handle index into table (driver-dependent)
	GPU_ShaderFS(const void* driverData = nullptr);
};

struct GPU_ShaderResource {
	sptr		m_driverData_view;

	GPU_ShaderResource() {
		m_driverData_view	= 0;
	}
};

struct GPU_TextureResource2D : public GPU_ShaderResource {
	sptr		m_driverData_tex;

	GPU_TextureResource2D() {
		m_driverData_tex	= 0;
		m_driverData_view	= 0;
	}
};

struct GPU_RenderTarget {
	sptr		m_driverData;		// can be either memory pointer or handle index into table (driver-dependent)
	GPU_RenderTarget(const void* driverData = nullptr);
};


inline GPU_ShaderVS::GPU_ShaderVS(const void* driverData) {
	m_driverData = (s64)driverData;
}

inline GPU_ShaderFS::GPU_ShaderFS(const void* driverData) {
	m_driverData = (s64)driverData;
}

inline GPU_VertexBuffer::GPU_VertexBuffer(const void* driverData) {
	m_driverData = (s64)driverData;
}

inline GPU_IndexBuffer::GPU_IndexBuffer(const void* driverData) {
	m_driverData = (s64)driverData;
}

inline GPU_RenderTarget::GPU_RenderTarget(const void* driverData) {
	m_driverData = (s64)driverData;
}

extern void					dx11_InitDevice();
extern void					dx11_CleanupDevice();

extern void					dx11_BackbufferSwap				();
extern int					dx11_CreateDynamicVertexBuffer	(int bufferSizeInBytes);
extern GPU_VertexBuffer		dx11_CreateStaticMesh			(void* vertexData, int itemSizeInBytes, int vertexCount);
extern GPU_IndexBuffer		dx11_CreateIndexBuffer			(void* indexBuffer, int bufferSize);
extern void					dx11_CreateTexture2D			(GPU_TextureResource2D& dest, const void* src_bitmap_data, int width, int height, GPU_ResourceFmt format);

extern void					dx11_BindShaderVS				(GPU_ShaderVS& vs);
extern void					dx11_BindShaderFS				(GPU_ShaderFS& fs);
extern void					dx11_SetVertexBuffer			(int bufferId, int shaderSlot, int _stride, int _offset);
extern void					dx11_UploadDynamicBufferData	(int bufferIdx, void* srcData, int sizeInBytes);
extern void					dx11_SetIndexBuffer				(GPU_IndexBuffer indexBuffer, int bitsPerIndex, int offset);
extern void					dx11_DrawIndexed				(int indexCount, int startIndexLoc, int baseVertLoc);
extern void					dx11_SetPrimType				(GpuPrimitiveType primType);

extern void					dx11_SetRasterState				(GpuRasterFillMode fill, GpuRasterCullMode cull, GpuRasterScissorMode scissor);
extern void					dx11_SetVertexBuffer			(const GPU_VertexBuffer& vbuffer, int shaderSlot, int _stride, int _offset);
extern void					dx11_BindShaderResource			(const GPU_ShaderResource& res, int startSlot=0);

extern bool					dx11_LoadShaderVS				(GPU_ShaderVS& dest, const xString& srcfile, const char* entryPointFn, GPU_VertexBufferLayoutType layoutType);
extern bool					dx11_LoadShaderFS				(GPU_ShaderFS& dest, const xString& srcfile, const char* entryPointFn);

extern void					dx11_ClearRenderTarget			(const GPU_RenderTarget& target, const float4& color);


extern bool					g_gpu_ForceWireframe;



extern GPU_RenderTarget		g_gpu_BackBuffer;