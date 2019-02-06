#pragma once

#include "x-types.h"
#include "x-simd.h"
#include "x-BitmapData.h"
#include "x-stdlib.h"

enum GPU_ResourceFmt : u8
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
    GPU_PRIM_POINTLIST      = 1,
    GPU_PRIM_LINELIST       = 2,
    GPU_PRIM_LINESTRIP      = 3,
    GPU_PRIM_TRIANGLELIST   = 4,
    GPU_PRIM_TRIANGLESTRIP  = 5,

    // DX11 doesn't support TriFan natively, but PS4 HW does.
    // Maybe TriFan topology can be set and then submitted indexes converted automatically to suit DX11 limitations?
};

enum GpuRasterFillMode {
    GPU_Fill_Solid      = 0,
    GPU_Fill_Wireframe  = 1,
    _GPU_Fill_Count_,
};

enum GpuRasterCullMode {
    GPU_Cull_None           = 0,
    GPU_Cull_Front          = 1,
    GPU_Cull_Back           = 2,
    _GPU_Cull_Count_
};

enum GpuRasterScissorMode {
    GPU_Scissor_Disable     = 0,
    GPU_Scissor_Enable      = 1,
    _GPU_Scissor_Count_
};

struct GPU_VertexBuffer {
    sptr        m_driverData    = 0;        // can be either memory pointer or handle index into table (driver-dependent)
    void Dispose();
};

struct GPU_IndexBuffer {
    sptr        m_driverData    = 0;        // can be either memory pointer or handle index into table (driver-dependent)
};

struct GPU_ConstantBuffer {
    sptr        m_driverData    = 0;        // can be either memory pointer or handle index into table (driver-dependent)
};

struct GPU_ShaderVS {
    sptr        m_driverBinary  = 0;        // fast-reference to shader binary
    sptr        m_driverBlob    = 0;        // slow-reference to secondary blob/debug information
};

struct GPU_ShaderFS {
    sptr        m_driverBinary  = 0;        // can be either memory pointer or handle index into table (driver-dependent)
    sptr        m_driverBlob    = 0;        // slow-reference to secondary blob/debug information
};

// Dynamic vertex buffers are multi-instanced, with one bound to each backbuffer in the swap chain.
// This allows the GameplaySceneLogic() system to update vertex buffers without blocking against draw operations
// being performed on the previous scene.
struct GPU_DynVsBuffer {
    int     m_buffer_idx;
    explicit GPU_DynVsBuffer(int m_buffer_idx = -1);

    bool IsValid() const { return m_buffer_idx >= 0; }
};

struct GPU_ShaderResource {
    sptr        m_driverData_view;

    GPU_ShaderResource() {
        m_driverData_view   = 0;
    }
};

struct GPU_TextureResource2D : public GPU_ShaderResource {
    sptr        m_driverData_tex;

    GPU_TextureResource2D() {
        m_driverData_tex    = 0;
        m_driverData_view   = 0;
    }
};

struct GPU_RenderTarget {
    sptr        m_driverData;       // can be either memory pointer or handle index into table (driver-dependent)
    GPU_RenderTarget(const void* driverData = nullptr);
};

// TODO : Create assertion macro that either throws exception or log-aborts depending on execution context.
//   * If running inside LUA scope, throw exception.
//   * If running outside LUA scope, immediate assert (VC++ debugger).


static const int MaxSemanticNameLen     = 16;       // includes null terminator
static const int MaxElementsPerSlot     = 8;
static const int InputLayoutMaxSlots    = 8;

struct InputLayoutItem
{
    char            SemanticName[MaxSemanticNameLen];
    GPU_ResourceFmt Format;
};

struct InputLayoutItemEx
{
    char            SemanticName[MaxSemanticNameLen];
    GPU_ResourceFmt Format;
    int             offset;
};


struct InputLayoutSlotItem
{
    char            SemanticName[MaxSemanticNameLen];
    s16             ByteOffset;
    GPU_ResourceFmt Format;
};

struct InputLayoutSlot
{
    int                 m_InstanceDataStepRate;     // non-zero indicates per-instance data
    int                 m_numElements;
    InputLayoutSlotItem m_Items [MaxElementsPerSlot];

    InputLayoutSlot&    SetInstanceStepRate(int step);
    InputLayoutSlot&    Append(const char* semanticName, GPU_ResourceFmt format, int offset=-1);
    InputLayoutSlot&    Append(const InputLayoutItem&   item);
    InputLayoutSlot&    Append(const InputLayoutItemEx& item);

    bool IsInstanceData() const { return m_InstanceDataStepRate != 0; }
};

using GPU_InputDescHash_t = u64;

class GPU_InputDesc
{
protected:
    InputLayoutSlot     m_slots[InputLayoutMaxSlots] = {};
    int                 m_numSlots      = 0;
    u64                 m_hashval       = 0;

public:
    void Reset() {
        xMemZero(*this);
    }

public:
    __ai u64                        GetHash         ()          const   { return m_hashval; }
    __ai int                        GetSlotCount    ()          const   { return m_numSlots; }
         const  InputLayoutSlot&    GetSlot         (int idx)   const   ;
                InputLayoutSlot&    GetSlot         (int idx)           ;

    template< int numItems > void AddVertexSlot     (const InputLayoutItem      (&items)[numItems])                             { _AddHash(_AddGenericSlot(items).SetInstanceStepRate(0)                    );  };
    template< int numItems > void AddVertexSlotEx   (const InputLayoutItemEx    (&items)[numItems])                             { _AddHash(_AddGenericSlot(items).SetInstanceStepRate(0)                    );  };
    template< int numItems > void AddInstanceSlot   (const InputLayoutItem      (&items)[numItems], int instanceDataStepRate=1) { _AddHash(_AddGenericSlot(items).SetInstanceStepRate(instanceDataStepRate) );  };
    template< int numItems > void AddInstanceSlotEx (const InputLayoutItemEx    (&items)[numItems], int instanceDataStepRate=1) { _AddHash(_AddGenericSlot(items).SetInstanceStepRate(instanceDataStepRate) );  };

protected:
    void _AddHash(const InputLayoutSlot& src);
    template< typename T, int numItems > InputLayoutSlot& _AddGenericSlot(const T (&items)[numItems]);
};

template< typename T, int numItems > InputLayoutSlot& GPU_InputDesc::_AddGenericSlot(const T (&items)[numItems])
{
    //throw_abort_on(m_numSlots >= InputLayoutMaxSlots);

    auto& newSlot = m_slots[m_numSlots];
    for (int i=0; i<numItems; ++i) {
        newSlot.Append(items[i]);
    }

    m_numSlots += 1;
    return newSlot;
}


inline GPU_DynVsBuffer::GPU_DynVsBuffer(int idx) {
    m_buffer_idx = idx;
}

inline GPU_RenderTarget::GPU_RenderTarget(const void* driverData) {
    m_driverData = (s64)driverData;
}

extern void                 dx11_InitDevice                 ();
extern void                 dx11_CleanupDevice              ();

extern void                 dx11_NewFrame                   ();
extern void                 dx11_BeginFrameDrawing          ();
extern void                 dx11_SubmitFrameAndSwap         ();
extern void                 dx11_CreateDynamicVertexBuffer  (GPU_DynVsBuffer& dest, int bufferSizeInBytes);
extern void                 dx11_CreateStaticMesh           (GPU_VertexBuffer&  dest, void* vertexData, int itemSizeInBytes, int vertexCount);
extern void                 dx11_CreateIndexBuffer          (GPU_IndexBuffer&   dest, void* indexBuffer, int bufferSize);
extern void                 dx11_CreateConstantBuffer       (GPU_ConstantBuffer& dest, int bufferSize);
extern void                 dx11_CreateTexture2D            (GPU_TextureResource2D& dest, const xBitmapDataRO& bitmap, GPU_ResourceFmt format);
extern void                 dx11_CreateTexture2D            (GPU_TextureResource2D& dest, const void* src_bitmap_data, const int2& size, GPU_ResourceFmt format);
extern void                 dx11_CreateTexture2D            (GPU_TextureResource2D& dest, const void* src_bitmap_data, int width, int height, GPU_ResourceFmt format);
extern void                 dx11_UploadDynamicBufferData    (const GPU_DynVsBuffer& bufferIdx, const void* srcData, int sizeInBytes);
extern void                 dx11_UpdateConstantBuffer       (const GPU_ConstantBuffer& buffer, const void* data);

extern bool                 dx11_TryLoadShaderVS            (GPU_ShaderVS& dest, const xString& srcfile, const char* entryPointFn);
extern bool                 dx11_TryLoadShaderFS            (GPU_ShaderFS& dest, const xString& srcfile, const char* entryPointFn);
extern void                 dx11_LoadShaderVS               (GPU_ShaderVS& dest, const xString& srcfile, const char* entryPointFn);
extern void                 dx11_LoadShaderFS               (GPU_ShaderFS& dest, const xString& srcfile, const char* entryPointFn);
extern void                 dx11_SetInputLayout             (const GPU_InputDesc& layout);
extern void                 dx11_SetRasterState             (GpuRasterFillMode fill, GpuRasterCullMode cull, GpuRasterScissorMode scissor);

extern void                 dx11_BindConstantBuffer         (const GPU_ConstantBuffer& buffer, int startSlot);
extern void                 dx11_BindShaderResource         (const GPU_ShaderResource& res, int startSlot=0);
extern void                 dx11_BindShaderVS               (const GPU_ShaderVS& vs);
extern void                 dx11_BindShaderFS               (const GPU_ShaderFS& fs);
extern void                 dx11_SetVertexBuffer            (const GPU_DynVsBuffer&  vbuffer, int shaderSlot, int _stride, int _offset);
extern void                 dx11_SetVertexBuffer            (const GPU_VertexBuffer& vbuffer, int shaderSlot, int _stride, int _offset);
extern void                 dx11_SetIndexBuffer             (const GPU_IndexBuffer& indexBuffer, int bitsPerIndex, int offset);
extern void                 dx11_SetPrimType                (GpuPrimitiveType primType);

extern void                 dx11_ClearRenderTarget          (const GPU_RenderTarget& target, const float4& color);
extern void                 dx11_DrawIndexed                (int indexCount, int startIndexLoc, int baseVertLoc);
extern void                 dx11_DrawInstanced              (int vertsPerInstance, int instanceCount, int startVertLoc, int startInstanceLoc);
extern void                 dx11_DrawIndexedInstanced       (int indexesPerInstance, int instanceCount, int startIndex, int baseVertex, int startInstance);
extern void                 dx11_Draw                       (int indexCount, int startVertLoc);

extern void                 dx11_InputLayoutCache_DisposeAll();

extern bool                 g_gpu_ForceWireframe;
extern GPU_RenderTarget     g_gpu_BackBuffer;
extern int2                 g_client_size_pix;
extern float                g_client_aspect_ratio;
