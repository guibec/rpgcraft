#pragma once

#include "v-float.h"

// --------------------------------------------------------------------------------------
//  VertexBufferState  (templated struct)
// --------------------------------------------------------------------------------------
template<typename VertexType>			// SimpleVertex
struct VertexBufferState {
	int 			m_vidx;
	VertexType*		vertices;
};


extern float CalculateBezierPoint(float t, float p0, float p1, float p2, float p3);
extern void  GenList2D_BezierLine(vFloat2* dest, int numSubdivs, const vFloat2& center, const vFloat2(&xy)[4]);

template<typename VertexType> extern void SubDiv_BezierFan		(VertexBufferState<VertexType>& dest, int numSubdivs, const vFloat2& center, const vFloat2(&xy)[4]);
template<typename VertexType> extern void StripTool_FromLine	(VertexBufferState<VertexType>& dest, const vFloat2 *xy, int numXY);
template<typename VertexType> extern void StripTool_FromPatch	(VertexBufferState<VertexType>& dest, const vFloat2 *xy, int numXY);


#define ImplVertBufferTmpl_BezierFan(type) \
	template void SubDiv_BezierFan<type>(VertexBufferState<type>& dest, int numSubdivs, const vFloat2& center, const vFloat2(&xy)[4]);

