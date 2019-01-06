
#include "PCH-framework.h"
#include "x-simd.h"

xString xDecStr(const int2& src)
{
    return xFmtStr("%d,%d", src.x, src.y);
}

xString xDecStr(const int4& src)
{
    return xFmtStr("%d,%d", src.x, src.y);
}

xString xDecStr(const uint2& src)
{
    return xFmtStr("%u,%u", src.x, src.y);
}

xString xHexStr(const int2& src)
{
    return xFmtStr("0x%x,0x%x", src.x, src.y);
}

xString xHexStr(const int4& src)
{
    return xFmtStr("0x%x,0x%x", src.x, src.y);
}

xString xHexStr(const uint2& src)
{
    return xFmtStr("0x%x,0x%x", src.x, src.y);
}
