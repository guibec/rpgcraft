
#pragma once

#include "x-gpu-ifc.h"
#include "x-simd.h"


// Standard colors (Red/Green/Blue/Alpha)
// Taken from DX11 header and converted to use a cross-platform neutral type.
// Also refactored to use an h/inl combination, in case we need to deal with some toolchain that lacks
// features like comdat elimination and/or LTO.

struct GPU_Colors {
	static const f128x4 AliceBlue            ;
	static const f128x4 AntiqueWhite         ;
	static const f128x4 Aqua                 ;
	static const f128x4 Aquamarine           ;
	static const f128x4 Azure                ;
	static const f128x4 Beige                ;
	static const f128x4 Bisque               ;
	static const f128x4 Black                ;
	static const f128x4 BlanchedAlmond       ;
	static const f128x4 Blue                 ;
	static const f128x4 BlueViolet           ;
	static const f128x4 Brown                ;
	static const f128x4 BurlyWood            ;
	static const f128x4 CadetBlue            ;
	static const f128x4 Chartreuse           ;
	static const f128x4 Chocolate            ;
	static const f128x4 Coral                ;
	static const f128x4 CornflowerBlue       ;
	static const f128x4 Cornsilk             ;
	static const f128x4 Crimson              ;
	static const f128x4 Cyan                 ;
	static const f128x4 DarkBlue             ;
	static const f128x4 DarkCyan             ;
	static const f128x4 DarkGoldenrod        ;
	static const f128x4 DarkGray             ;
	static const f128x4 DarkGreen            ;
	static const f128x4 DarkKhaki            ;
	static const f128x4 DarkMagenta          ;
	static const f128x4 DarkOliveGreen       ;
	static const f128x4 DarkOrange           ;
	static const f128x4 DarkOrchid           ;
	static const f128x4 DarkRed              ;
	static const f128x4 DarkSalmon           ;
	static const f128x4 DarkSeaGreen         ;
	static const f128x4 DarkSlateBlue        ;
	static const f128x4 DarkSlateGray        ;
	static const f128x4 DarkTurquoise        ;
	static const f128x4 DarkViolet           ;
	static const f128x4 DeepPink             ;
	static const f128x4 DeepSkyBlue          ;
	static const f128x4 DimGray              ;
	static const f128x4 DodgerBlue           ;
	static const f128x4 Firebrick            ;
	static const f128x4 FloralWhite          ;
	static const f128x4 ForestGreen          ;
	static const f128x4 Fuchsia              ;
	static const f128x4 Gainsboro            ;
	static const f128x4 GhostWhite           ;
	static const f128x4 Gold                 ;
	static const f128x4 Goldenrod            ;
	static const f128x4 Gray                 ;
	static const f128x4 Green                ;
	static const f128x4 GreenYellow          ;
	static const f128x4 Honeydew             ;
	static const f128x4 HotPink              ;
	static const f128x4 IndianRed            ;
	static const f128x4 Indigo               ;
	static const f128x4 Ivory                ;
	static const f128x4 Khaki                ;
	static const f128x4 Lavender             ;
	static const f128x4 LavenderBlush        ;
	static const f128x4 LawnGreen            ;
	static const f128x4 LemonChiffon         ;
	static const f128x4 LightBlue            ;
	static const f128x4 LightCoral           ;
	static const f128x4 LightCyan            ;
	static const f128x4 LightGoldenrodYellow ;
	static const f128x4 LightGreen           ;
	static const f128x4 LightGray            ;
	static const f128x4 LightPink            ;
	static const f128x4 LightSalmon          ;
	static const f128x4 LightSeaGreen        ;
	static const f128x4 LightSkyBlue         ;
	static const f128x4 LightSlateGray       ;
	static const f128x4 LightSteelBlue       ;
	static const f128x4 LightYellow          ;
	static const f128x4 Lime                 ;
	static const f128x4 LimeGreen            ;
	static const f128x4 Linen                ;
	static const f128x4 Magenta              ;
	static const f128x4 Maroon               ;
	static const f128x4 MediumAquamarine     ;
	static const f128x4 MediumBlue           ;
	static const f128x4 MediumOrchid         ;
	static const f128x4 MediumPurple         ;
	static const f128x4 MediumSeaGreen       ;
	static const f128x4 MediumSlateBlue      ;
	static const f128x4 MediumSpringGreen    ;
	static const f128x4 MediumTurquoise      ;
	static const f128x4 MediumVioletRed      ;
	static const f128x4 MidnightBlue         ;
	static const f128x4 MintCream            ;
	static const f128x4 MistyRose            ;
	static const f128x4 Moccasin             ;
	static const f128x4 NavajoWhite          ;
	static const f128x4 Navy                 ;
	static const f128x4 OldLace              ;
	static const f128x4 Olive                ;
	static const f128x4 OliveDrab            ;
	static const f128x4 Orange               ;
	static const f128x4 OrangeRed            ;
	static const f128x4 Orchid               ;
	static const f128x4 PaleGoldenrod        ;
	static const f128x4 PaleGreen            ;
	static const f128x4 PaleTurquoise        ;
	static const f128x4 PaleVioletRed        ;
	static const f128x4 PapayaWhip           ;
	static const f128x4 PeachPuff            ;
	static const f128x4 Peru                 ;
	static const f128x4 Pink                 ;
	static const f128x4 Plum                 ;
	static const f128x4 PowderBlue           ;
	static const f128x4 Purple               ;
	static const f128x4 Red                  ;
	static const f128x4 RosyBrown            ;
	static const f128x4 RoyalBlue            ;
	static const f128x4 SaddleBrown          ;
	static const f128x4 Salmon               ;
	static const f128x4 SandyBrown           ;
	static const f128x4 SeaGreen             ;
	static const f128x4 SeaShell             ;
	static const f128x4 Sienna               ;
	static const f128x4 Silver               ;
	static const f128x4 SkyBlue              ;
	static const f128x4 SlateBlue            ;
	static const f128x4 SlateGray            ;
	static const f128x4 Snow                 ;
	static const f128x4 SpringGreen          ;
	static const f128x4 SteelBlue            ;
	static const f128x4 Tan                  ;
	static const f128x4 Teal                 ;
	static const f128x4 Thistle              ;
	static const f128x4 Tomato               ;
	static const f128x4 Transparent          ;
	static const f128x4 Turquoise            ;
	static const f128x4 Violet               ;
	static const f128x4 Wheat                ;
	static const f128x4 White                ;
	static const f128x4 WhiteSmoke           ;
	static const f128x4 Yellow               ;
	static const f128x4 YellowGreen          ;
};

#if !TOOLCHAIN_LTO_ENABLED
#	include "x-gpu-colors.inl"
#endif