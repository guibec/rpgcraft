#pragma once

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


