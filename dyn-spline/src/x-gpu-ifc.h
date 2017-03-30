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

struct GPU_IndexBuffer {
	s64		m_driverData;		// can be either memory pointer or handle index into table (driver-dependent)
	GPU_IndexBuffer(const void* driverData = nullptr);
};

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


