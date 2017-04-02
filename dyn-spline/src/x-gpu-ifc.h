#pragma once

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
	s64		m_driverData;		// can be either memory pointer or handle index into table (driver-dependent)
	GPU_VertexBuffer(const void* driverData = nullptr);
};

struct GPU_IndexBuffer {
	s64		m_driverData;		// can be either memory pointer or handle index into table (driver-dependent)
	GPU_IndexBuffer(const void* driverData = nullptr);
};

inline GPU_VertexBuffer::GPU_VertexBuffer(const void* driverData) {
	m_driverData = (s64)driverData;
}

inline GPU_IndexBuffer::GPU_IndexBuffer(const void* driverData) {
	m_driverData = (s64)driverData;
}


extern void				dx11_BackbufferSwap				();
extern int				dx11_CreateDynamicVertexBuffer	(int bufferSizeInBytes);
extern GPU_IndexBuffer	dx11_CreateIndexBuffer			(void* indexBuffer, int bufferSize);
extern void				dx11_SetVertexBuffer			(int bufferId, int shaderSlot, int _stride, int _offset);
extern void				dx11_UploadDynamicBufferData	(int bufferIdx, void* srcData, int sizeInBytes);
extern void				dx11_SetIndexBuffer				(GPU_IndexBuffer indexBuffer, int bitsPerIndex, int offset);
extern void				dx11_SetPrimType				(GpuPrimitiveType primType);

extern void				dx11_SetRasterState				(GpuRasterFillMode fill, GpuRasterCullMode cull, GpuRasterScissorMode scissor);

extern void				dx11_SetVertexBuffer			(const GPU_VertexBuffer& vbuffer, int shaderSlot, int _stride, int _offset);
extern GPU_VertexBuffer	dx11_CreateStaticMesh			(void* vertexData, int itemSizeInBytes, int vertexCount);

extern bool				g_gpu_ForceWireframe;
