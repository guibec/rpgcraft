#pragma once

#include "x-BitmapData.h"

struct rgba32 {
    u8   a,b,g,r;
    rgba32(u8 r_, u8 g_, u8 b_, u8 a_) {
        a = a_;
        b = b_;
        g = g_;
        r = r_;
    }
    rgba32(u32 im_code) {
        // im_code expected format is ARGB, which is convenient for annotation when the alpha component
        // is ineffective (ignored).

        a = (im_code >> 24) & 0xff;
        b = (im_code >>  0) & 0xff;
        g = (im_code >>  8) & 0xff;
        r = (im_code >> 16) & 0xff;

    }
    u32 w32() const {
        return (u32&)(*this);
    }
};

namespace imgtool
{
    static const u32 mask_alpha = 0x000000ff;
    static const u32 mask_rgb   = 0xffffff00;

    void        ConvertOpaqueColorToAlpha               (xBitmapData& image, const rgba32& color);
    void        CutTex                                  (xBitmapData& dest, const xBitmapData& src, int2 xy1, int2 xy2);
    void        CutTex_and_ConvertOpaqueColorToAlpha    (xBitmapData& dest, const xBitmapData& src, int2 xy1, int2 xy2, const rgba32& color);
    int         AddTileToAtlas                          (TextureAtlas& dest, const xBitmapData& src, const int2& srcpos = {0,0});
    int         AddEmptyTileToAtlas                     (TextureAtlas& dest, const float4& color={0,0,0,1.0f});
};

namespace DbgFont
{
    static const int        BytesPerGlyph = 8;
    extern const int        LowCharacterCode;
    extern const int        CharacterCodeCount;
    extern const int        HighCharacterCode;
    extern const uint8_t    GlyphData[][BytesPerGlyph];

    extern int      BlitChar            (xBitmapData& dest, int2 topleft, char ch, const float4& color={1.f,1.f,1.f,1.f});
    extern void     BlitAtlasTexture    (xBitmapData& dest);
};

