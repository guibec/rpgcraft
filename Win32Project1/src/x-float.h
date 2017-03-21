#pragma once

#include "x-simd.h"

union	xFloat4;
struct	xFloat3;
union	xFloat2;

union xFloat4
{
    struct {
        float	x;
        float	y;
        float	z;
        float	w;
    };

    u128        q;

    xFloat4() {}
    
    xFloat4(float _x, float _y,  float _z, float _w);
    xFloat4(float _x, float _y,  xFloat2 _zw);
    xFloat4(xFloat3 _xyz, float _w);
    xFloat4(xFloat2 _xy,  float _z, float _w);
    xFloat4(xFloat2 _xy,  xFloat2 _zw);
};

struct xFloat3
{
    float		x;
    float		y;
    float		z;

    xFloat3() {}

    xFloat3(float _x, float _y, float _z);
    xFloat3(xFloat2 _xy, float _z);
};

union xFloat2
{
    struct {
        float	x;
        float	y;
    };

    struct {
        s32		xi_raw;
        s32		yi_raw;
    };

    u64			d;

    xFloat2() {}

    xFloat2(float _x, float _y);
};

inline xFloat4::xFloat4(float _x, float _y, float _z, float _w) {
    x = _x;
    y = _y;
    z = _z;
    w = _w;
}

inline xFloat4::xFloat4(xFloat2 _xy,  float _z, float _w) {
    x = _xy.x;
    y = _xy.y;
    z = _z;
    w = _w;
}

inline xFloat4::xFloat4(xFloat2 _xy,  xFloat2 _zw) {
    x = _xy.x;
    y = _xy.y;
    z = _zw.x;
    w = _zw.y;
}

inline xFloat4::xFloat4(float _x, float _y,  xFloat2 _zw) {
    x = _x;
    y = _y;
    z = _zw.x;
    w = _zw.y;
}

inline xFloat4::xFloat4(xFloat3 _xyz, float _w) {
    x = _xyz.x;
    y = _xyz.y;
    z = _xyz.z;
    w = _w;
}

inline xFloat3::xFloat3( float _x, float _y, float _z) {
    x = _x;
    y = _y;
    z = _z;
}

inline xFloat3::xFloat3( xFloat2 _xy, float _z) {
    x = _xy.x;
    y = _xy.y;
    z = _z;
}

inline xFloat2::xFloat2( float _x, float _y) {
    x = _x;
    y = _y;
}


struct SimpleVertex
{
    xFloat3 Pos;
    //XMFLOAT4 Color;
};
