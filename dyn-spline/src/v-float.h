#pragma once

#include "x-simd.h"

// =================================================================================
// v-float.h - PACKING IS CRITICAL
// =================================================================================
// Entire point of these structures is to define vertex information for passing into vertex shaders.
// Packing is critical.  When working with data on CPU, use u128 as much as possible, and then
// shuffle-pack data into vFloat for backend processing.

__pragma(pack(push,1))

struct 	vFloat4;
struct	vFloat3;
struct	vFloat2;


struct vFloat4
{
	float	x;
	float	y;
	float	z;
	float	w;

	vFloat4() {}
	vFloat4(float _x, float _y,  float _z, float _w);
	vFloat4(float _x, float _y,  vFloat2 _zw);
	vFloat4(vFloat3 _xyz, float _w);
	vFloat4(vFloat2 _xy,  float _z, float _w);
	vFloat4(vFloat2 _xy,  vFloat2 _zw);

	operator u128() const;
};

inline vFloat4::operator u128()	const { return *(u128*)this; }

struct vFloat3
{
	float		x;
	float		y;
	float		z;

	vFloat3() {}

	vFloat3(float _x, float _y, float _z);
	vFloat3(vFloat2 _xy, float _z);
};

struct vFloat2
{
	float	x;
	float	y;

	vFloat2() {}
	vFloat2(float _x, float _y);
};

inline vFloat4::vFloat4(float _x, float _y, float _z, float _w) {
	x = _x;
	y = _y;
	z = _z;
	w = _w;
}

inline vFloat4::vFloat4(vFloat2 _xy,  float _z, float _w) {
	x = _xy.x;
	y = _xy.y;
	z = _z;
	w = _w;
}

inline vFloat4::vFloat4(vFloat2 _xy,  vFloat2 _zw) {
	x = _xy.x;
	y = _xy.y;
	z = _zw.x;
	w = _zw.y;
}

inline vFloat4::vFloat4(float _x, float _y,  vFloat2 _zw) {
	x = _x;
	y = _y;
	z = _zw.x;
	w = _zw.y;
}

inline vFloat4::vFloat4(vFloat3 _xyz, float _w) {
	x = _xyz.x;
	y = _xyz.y;
	z = _xyz.z;
	w = _w;
}

inline vFloat3::vFloat3(float _x, float _y, float _z) {
	x = _x;
	y = _y;
	z = _z;
}

inline vFloat3::vFloat3(vFloat2 _xy, float _z) {
	x = _xy.x;
	y = _xy.y;
	z = _z;
}

inline vFloat2::vFloat2(float _x, float _y) {
	x = _x;
	y = _y;
}


struct SimpleVertex
{
	vFloat3 Pos;
	vFloat4 Color;
} __packed;


__pragma(pack(pop))
