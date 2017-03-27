#pragma once

#include "Bezier2D.h"

// --------------------------------------------------------------------------------------
//  SubDiv_BezierFan  ()
// --------------------------------------------------------------------------------------
// Thoughts:
//  - Steps per side should be a function of the length of a side.
//  - Visible length is also a function of the distance from the camera, in case there's some zoom or 3D effect happening.
//  - The length can vary according to the control points:
//
//       chord = (p3-p0).Length;
//       cont_net = (p0 - p1).Length + (p2 - p1).Length + (p3 - p2).Length;
//       app_arc_length = (cont_net + chord) / 2;
//
// To calculate perpendicular angle along a curve, use standard tangent method:
//    ATAN2(DY, DX) +/- Math.Pi/2
//    ATAN2((pn+1).y - (pn-1).y, (pn+1).x - (pn-1).x) +/- Math.Pi/2
//
// To convert angle into a line:
//     x = start_x + len * cos(angle);		// startx == pn.x
//	   y = start_y + len * sin(angle);		// starty == pn.y
//
// Long-tail TODO:  Try implementing this as a geometry shader?
//     Pro: Can determine correct # of verticies based on length within actual scene orientation.
//     Con: Geometry shaders are still generally slower than just rough-guessing lengths based on
//          distance from player.
//

template<typename VertexType>
void SubDiv_BezierFan(VertexBufferState<VertexType>& dest, int numSubdivs, const vFloat2& center, const vFloat2(&xy)[4])
{
	const auto& x1 = xy[0].x;
	const auto& x2 = xy[1].x;
	const auto& x3 = xy[2].x;
	const auto& x4 = xy[3].x;

	const auto& y1 = xy[0].y;
	const auto& y2 = xy[1].y;
	const auto& y3 = xy[2].y;
	const auto& y4 = xy[3].y;

	float stepIncr = 1.0f / numSubdivs;

	for (int idx = 0; idx < numSubdivs; ++idx)
	{
		float i = idx * stepIncr;

		float x = CalculateBezierPoint(i, x1, x2, x3, x4);
		float y = CalculateBezierPoint(i, y1, y2, y3, y4);

		dest.vertices[dest.m_vidx].Pos = vFloat3(x, y, 0.5f);
		dest.m_vidx += 1;
	}
}

// StripTool_FromLine
// StripTool_FromPatch
//
// Converts a series of 2D xy coordinates into a triangle strip with a given 2D thickness.
// FromLine  assumes endpoints are independent.
//           (numXY-1)*2 triangles will be generated.
//
// FromPatch assumes endpoints are closed.
//           (numXY  )*2 triangles will be generated.

template<typename VertexType>
void StripTool_FromLine(VertexBufferState<VertexType>& dest, const vFloat2 *xy, int numXY)
{

//    ATAN2((pn+1).y - (pn-1).y, (pn+1).x - (pn-1).x) +/- Math.Pi/2
	
}

template<typename VertexType>
void StripTool_FromPatch(VertexBufferState<VertexType>& dest, const vFloat2 *xy, int numXY)
{

//    ATAN2((pn+1).y - (pn-1).y, (pn+1).x - (pn-1).x) +/- Math.Pi/2
	
}
