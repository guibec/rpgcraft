#include "PCH-rpgcraft.h"

#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"
#include "x-gpu-ifc.h"
#include "x-gpu-colors.h"

#include "Bezier2D.h"

#include "Bezier2d.inl"

static const int const_zval = 0.1f;

static const int        numStepsPerCurve        = 10;
static const int        numTrisPerCurve         =  numStepsPerCurve;
static const int        numVertexesPerCurve     = (numTrisPerCurve * 3);
static const int        SimpleVertexBufferSize  = (numTrisPerCurve*4) + 1;

GPU_DynVsBuffer         g_VertexBufferId;
GPU_IndexBuffer         g_IndexBuffer;


void PopulateIndices_TriFan(s16* dest, int& iidx, int& vertexIdx, int numSubdivs)
{
    for (int idx = 0; idx < numSubdivs; ++idx)
    {
        // non-interpolated (flat shading) determines color according the first vertex.
        // To allow for non-solid colorization of the objectm make sure the first vertex
        // is *not* the center point:

        dest[iidx + 0] = vertexIdx + 0;
        dest[iidx + 1] = 0;
        dest[iidx + 2] = vertexIdx + 1;
        vertexIdx   += 1;
        iidx        += 3;
    }
}

// TripStrip is natively supported by all GPUs, so providing an index list is not necessary.
// This function may be useful for some fancier mesh types though, where it consists of some
// mix of fan and strip triangles, etc...
void PopulateIndices_TriStrip(s16* dest, int& iidx, int& vertexIdx, int numSubdivs)
{
    // Expected vertex layout:
    //   vidx 0   - Edge 0
    //   vidx 1,2 - Center, Edge 1
    //   vidx 3,4 - Center, Edge 2
    //   vidx 5,6 - Center, Edge 3

    bug_on(vertexIdx==0, "vertexIdx must be at least 1; vertexIdx-1 should be first vertex along outside edge.");

    for (int idx = 0; idx < numSubdivs; ++idx)
    {
        dest[iidx + 0] = vertexIdx - 1;
        dest[iidx + 1] = vertexIdx;
        dest[iidx + 2] = vertexIdx + 1;
        vertexIdx   += 2;
        iidx        += 3;
    }
}

void SplineTest_Init()
{
    s16    indices [(numVertexesPerCurve*4)];

    int iidx = 0;
    int vidx = 1;

    PopulateIndices_TriFan(indices, iidx, vidx, numStepsPerCurve);
    PopulateIndices_TriFan(indices, iidx, vidx, numStepsPerCurve);
    PopulateIndices_TriFan(indices, iidx, vidx, numStepsPerCurve);
    PopulateIndices_TriFan(indices, iidx, vidx, numStepsPerCurve);

    // Close the patch by creating triangle between last vertex along the spline and first one.

    indices[(iidx-3)+1] = 0;
    indices[(iidx-3)+0] = vidx-1;
    indices[(iidx-3)+2] = 1;

    assume(vidx <= SimpleVertexBufferSize);
    assume(iidx <= bulkof(indices));

    dx11_CreateDynamicVertexBuffer(g_VertexBufferId, sizeof(SimpleVertex) * SimpleVertexBufferSize);
    dx11_CreateIndexBuffer(g_IndexBuffer, indices, sizeof(indices));
}


void SplineTest_Render()
{
    const vFloat2 top1[4] = {
        {  -0.5f,  -0.5f },
        {  -0.2f,  -0.3f },     // control point
        {   0.3f,  -0.6f },     // control point
        {   0.5f,  -0.5f },
    };

    const vFloat2 left1[4] = {
        {   0.5f,  -0.5f },
        {   0.8f,  -0.2f },     // control point
        {   0.8f,   0.2f },     // control point
        {   0.7f,   0.6f },
    };

    const vFloat2 bottom1[4] = {
        {   0.7f,   0.6f },
        {   0.3f,   0.3f },     // control point
        {  -0.2f,   0.9f },     // control point
        {  -0.5f,   0.5f },
    };

    const vFloat2 right1[4] = {
        {  -0.5f,   0.5f },
        {  -0.8f,   0.2f },     // control point
        {  -0.8f,  -0.2f },     // control point
        {  -0.5f,  -0.5f },
    };


    // --------------------------------------------------------------------------------------

    const vFloat2 top2[4] = {
        {  -0.3f,  -0.5f },
        {  -0.2f,  -0.3f },     // control point
        {   0.3f,  -0.6f },     // control point
        {   0.5f,  -0.3f },
    };

    const vFloat2 left2[4] = {
        {   0.5f,  -0.3f },
        {   0.8f,  -0.4f },     // control point
        {   0.8f,   0.2f },     // control point
        {   0.5f,   0.5f },
    };

    const vFloat2 bottom2[4] = {
        {   0.5f,   0.5f },
        {   0.3f,   0.3f },     // control point
        {  -0.2f,   0.7f },     // control point
        {  -0.5f,   0.5f },
    };

    const vFloat2 right2[4] = {
        {  -0.5f,   0.5f },
        {  -0.8f,   0.2f },     // control point
        {  -0.9f,  -0.2f },     // control point
        {  -0.3f,  -0.5f },
    };

    static float animPos = 0.0f;

    animPos += 0.06f;
    animPos = std::fmodf(animPos, 3.14159f * 4);

    float apos = (1.0f + std::sinf(animPos)) * 0.5f;

#define lerp(a,b)       ((a) + (((b)-(a))* (apos)))

#define lerpBezier(varA, varB)  { \
    { ((varA)[0].x + (((varB)[0].x-(varA)[0].x) * (apos))), ((varA)[0].y + (((varB)[0].y-(varA)[0].y) * (apos))) },     \
    { ((varA)[1].x + (((varB)[1].x-(varA)[1].x) * (apos))), ((varA)[1].y + (((varB)[1].y-(varA)[1].y) * (apos))) },     \
    { ((varA)[2].x + (((varB)[2].x-(varA)[2].x) * (apos))), ((varA)[2].y + (((varB)[2].y-(varA)[2].y) * (apos))) },     \
    { ((varA)[3].x + (((varB)[3].x-(varA)[3].x) * (apos))), ((varA)[3].y + (((varB)[3].y-(varA)[3].y) * (apos))) },     \
}

    vFloat2 top     [4] = lerpBezier(top1,      top2    );
    vFloat2 left    [4] = lerpBezier(left1,     left2   );
    vFloat2 bottom  [4] = lerpBezier(bottom1,   bottom2 );
    vFloat2 right   [4] = lerpBezier(right1,    right2  );

    SimpleVertex vertices[SimpleVertexBufferSize];

    vFloat2     center  = vFloat2(0.0f,     0.0f);

    vertices[0].Pos = vFloat3(center, const_zval);

    VertexBufferState<SimpleVertex> vstate = { 1, vertices };

    SubDiv_BezierFan(vstate, numStepsPerCurve, center, top);
    SubDiv_BezierFan(vstate, numStepsPerCurve, center, left);
    SubDiv_BezierFan(vstate, numStepsPerCurve, center, bottom);
    SubDiv_BezierFan(vstate, numStepsPerCurve, center, right);

    assume(vstate.m_vidx <= bulkof(vertices));

    // ------------------------------------------------------------------------------------------
    // Apply Diagnostic color for visualizing geometry...

    if (1) {
        static float colorYay = 0.0f;
        float colorIdx = colorYay; //0.0f;
        foreach( auto& vert, vertices ) {
            vert.Color = vFloat4( colorIdx, 0.0f, 0.0f, 1.0f );
            colorIdx  += 0.05f;
            colorIdx   = std::fmodf(colorIdx, 1.0f);
        }

        colorYay += 0.01f;
        vertices[0].Color.x = 0.0f;
        vertices[0].Color.y = 0.0f;
        vertices[0].Color.z = 0.0f;
    }

    // Clear the back buffer
    dx11_ClearRenderTarget(g_gpu_BackBuffer, GPU_Colors::MidnightBlue);

    // ------------------------------------------------------------------------------------------
    // Update Spline Vertex Information

    if (g_VertexBufferId.IsValid()) {
        dx11_UploadDynamicBufferData(g_VertexBufferId, vertices, sizeof(vertices));
    }

    dx11_SetIndexBuffer(g_IndexBuffer, 16, 0);
    dx11_SetVertexBuffer(g_VertexBufferId, 0, sizeof(SimpleVertex), 0);
    dx11_SetPrimType(GPU_PRIM_TRIANGLELIST);

}
