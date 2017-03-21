
#include "x-types.h"
#include "Bezier2D.h"

#include <cmath>		// for ATAN2 - TODO: replace with SIMD version


float CalculateBezierPoint(float t, float p0, float p1, float p2, float p3)
{
	float u		= 1.0f - t;
	float tt	=  t * t;
	float uu	=  u * u;
	float uuu	= uu * u;
	float ttt	= tt * t;

	float p;
	p =  1.0f * uuu *    1  * p0;	//first term
	p += 3.0f *  uu *    t  * p1;	//second term
	p += 3.0f *   u *   tt  * p2;	//third term
	p += 1.0f *   1 *  ttt  * p3;	//fourth term

	return p;
}

void GenList2D_BezierLine(xFloat2* dest, int numSubdivs, const xFloat2& center, const xFloat2(&xy)[4])
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

		dest[idx] = xFloat2(x, y);
	}
}

