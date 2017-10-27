#include "PCH-rpgcraft.h"

#include "x-types.h"
#include "x-stl.h"
#include "x-assertion.h"
#include "x-string.h"

#include "x-png-decode.h"
#include "x-gpu-ifc.h"
#include "v-float.h"

#include "UniformMeshes.h"
#include "DbgFont.h"
#include "Scene.h"

#include "ajek-script.h"

#include <vector>

#include <DirectXMath.h>

// Debug font uses a simplified version of tile rendering.
// A draw command is submitted with a grid of character information.
// The character IDs are used to look up the font information from a texture.
//
// DbgFont is divided into two APIs:
//   DbgFont
//     * Renders each character individually using static mesh (XY+UV) and constant buffer (position)
//     * Intended for positional text display, such as XY coords over NPC heads, or similar.
//   DbgFontSheet
//     * Renders a grid of characters using dynamic character map uploaded per-frame.
//     * Intended for use only for two specific usage cases, which are globally defined:
//	       1. universal overlay.
//	       2. drop-down console.
//
// Support list:
//   * Monospaced font only
//   * Scaling and rotation is supported
//   * DbgFont is not read-only during game Render stage - Information can be written
//     to a given font sheet at any time during both Logic and Render pipelines.


// ======================================================================================================
// Font Details:
//  * chars 0->127     are 6x8 -  5x8 with 1 column spacing.
//  * chars 128->152   are 8x8 - special chars (not ANSI) for drawing debug menu decorations.
//
// I'm tempted to separate them into two separate mappings.
// Benefits:
//   * Drawing the 6x8 font at 6x8 or 7x8 is more compact.  It's nice!
//   * Drawing menu decorations using ascii strings isn't my idea of fun, actually.  Might as well
//     have them in their own sheet with dedicated helper classes for constructing menu decoration geometry.
//
// Caveat:
//   * Having characters at 6x8 and decorations at 8x8 might make math a little more complicated.
//     But maybe not: horizontal and vertical spans can be shortened/clipped without any special logic.
//     (could even use repeating texture logic feature).  So really it's just a matter of knowing the
//     dims of the debugfont sheet and drawing a decoration around it to scale.
//
// Conclusion:
//   * Should probably separate them, and render debug font as 6x8 or 7x8!
// ======================================================================================================

static const int BytesPerGlyph = 8;

static const uint8_t font[][BytesPerGlyph] =
{
	{0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, // 0..7
	{0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,},	{0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, // 8..15
	{0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,},	{0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, // 16..23
	{0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,},	{0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, {0,0,0,0,0,0,0,0,}, // 24..31

	{ // 32
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
	},
	{ // 33
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x00, // ......
		0x20, // ..*...
		0x00, // ......
	},
	{ // 34
		0x50, // .*.*..
		0x50, // .*.*..
		0x50, // .*.*..
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
	},
	{ // 35
		0x50, // .*.*..
		0x50, // .*.*..
		0xf8, // *****.
		0x50, // .*.*..
		0xf8, // *****.
		0x50, // .*.*..
		0x50, // .*.*..
		0x00, // ......
	},
	{ // 36
		0x20, // ..*...
		0x78, // .****.
		0xa0, // *.*...
		0x70, // .***..
		0x28, // ..*.*.
		0xf0, // ****..
		0x20, // ..*...
		0x00, // ......
	},
	{ // 37
		0xc0, // **....
		0xc8, // **..*.
		0x10, // ...*..
		0x20, // ..*...
		0x40, // .*....
		0x98, // *..**.
		0x18, // ...**.
		0x00, // ......
	},
	{ // 38
		0x40, // .*....
		0xa0, // *.*...
		0xa0, // *.*...
		0x40, // .*....
		0xa8, // *.*.*.
		0x90, // *..*..
		0x68, // .**.*.
		0x00, // ......
	},
	{ // 39
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
	},
	{ // 40
		0x20, // ..*...
		0x40, // .*....
		0x80, // *.....
		0x80, // *.....
		0x80, // *.....
		0x40, // .*....
		0x20, // ..*...
		0x00, // ......
	},
	{ // 41
		0x20, // ..*...
		0x10, // ...*..
		0x08, // ....*.
		0x08, // ....*.
		0x08, // ....*.
		0x10, // ...*..
		0x20, // ..*...
		0x00, // ......
	},
	{ // 42
		0x20, // ..*...
		0xa8, // *.*.*.
		0x70, // .***..
		0xa8, // *.*.*.
		0x70, // .***..
		0xa8, // *.*.*.
		0x20, // ..*...
		0x00, // ......
	},
	{ // 43
		0x00, // ......
		0x20, // ..*...
		0x20, // ..*...
		0xf8, // *****.
		0x20, // ..*...
		0x20, // ..*...
		0x00, // ......
		0x00, // ......
	},
	{ // 44
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x20, // ..*...
		0x20, // ..*...
		0x40, // .*....
	},
	{ // 45
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0xf8, // *****.
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
	},
	{ // 46
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x20, // ..*...
		0x00, // ......
	},
	{ // 47
		0x00, // ......
		0x08, // ....*.
		0x10, // ...*..
		0x20, // ..*...
		0x40, // .*....
		0x80, // *.....
		0x00, // ......
		0x00, // ......
	},
	{ // 48
		0x70, // .***..
		0x88, // *...*.
		0x98, // *..**.
		0xa8, // *.*.*.
		0xc8, // **..*.
		0x88, // *...*.
		0x70, // .***..
		0x00, // ......
	},
	{ // 49
		0x20, // ..*...
		0x60, // .**...
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x70, // .***..
		0x00, // ......
	},
	{ // 50
		0x70, // .***..
		0x88, // *...*.
		0x08, // ....*.
		0x30, // ..**..
		0x40, // .*....
		0x80, // *.....
		0xf8, // *****.
		0x00, // ......
	},
	{ // 51
		0xf8, // *****.
		0x08, // ....*.
		0x10, // ...*..
		0x30, // ..**..
		0x08, // ....*.
		0x88, // *...*.
		0x70, // .***..
		0x00, // ......
	},
	{ // 52
		0x10, // ...*..
		0x30, // ..**..
		0x50, // .*.*..
		0x90, // *..*..
		0xf8, // *****.
		0x10, // ...*..
		0x10, // ...*..
		0x00, // ......
	},
	{ // 53
		0xf8, // *****.
		0x80, // *.....
		0xf0, // ****..
		0x08, // ....*.
		0x08, // ....*.
		0x88, // *...*.
		0x70, // .***..
		0x00, // ......
	},
	{ // 54
		0x38, // ..***.
		0x40, // .*....
		0x80, // *.....
		0xf0, // ****..
		0x88, // *...*.
		0x88, // *...*.
		0x70, // .***..
		0x00, // ......
	},
	{ // 55
		0xf8, // *****.
		0x08, // ....*.
		0x10, // ...*..
		0x20, // ..*...
		0x40, // .*....
		0x40, // .*....
		0x40, // .*....
		0x00, // ......
	},
	{ // 56
		0x70, // .***..
		0x88, // *...*.
		0x88, // *...*.
		0x70, // .***..
		0x88, // *...*.
		0x88, // *...*.
		0x70, // .***..
		0x00, // ......
	},
	{ // 57
		0x70, // .***..
		0x88, // *...*.
		0x88, // *...*.
		0x78, // .****.
		0x08, // ....*.
		0x10, // ...*..
		0xe0, // ***...
		0x00, // ......
	},
	{ // 58
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x20, // ..*...
		0x00, // ......
		0x20, // ..*...
		0x00, // ......
		0x00, // ......
	},
	{ // 59
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x20, // ..*...
		0x00, // ......
		0x20, // ..*...
		0x20, // ..*...
		0x40, // .*....
	},
	{ // 60
		0x10, // ...*..
		0x20, // ..*...
		0x40, // .*....
		0x80, // *.....
		0x40, // .*....
		0x20, // ..*...
		0x10, // ...*..
		0x00, // ......
	},
	{ // 61
		0x00, // ......
		0x00, // ......
		0xf8, // *****.
		0x00, // ......
		0xf8, // *****.
		0x00, // ......
		0x00, // ......
		0x00, // ......
	},
	{ // 62
		0x40, // .*....
		0x20, // ..*...
		0x10, // ...*..
		0x08, // ....*.
		0x10, // ...*..
		0x20, // ..*...
		0x40, // .*....
		0x00, // ......
	},
	{ // 63
		0x70, // .***..
		0x88, // *...*.
		0x10, // ...*..
		0x20, // ..*...
		0x20, // ..*...
		0x00, // ......
		0x20, // ..*...
		0x00, // ......
	},
	{ // 64
		0x70, // .***..
		0x88, // *...*.
		0xa8, // *.*.*.
		0xb8, // *.***.
		0xb0, // *.**..
		0x80, // *.....
		0x78, // .****.
		0x00, // ......
	},
	{ // 65
		0x20, // ..*...
		0x50, // .*.*..
		0x88, // *...*.
		0x88, // *...*.
		0xf8, // *****.
		0x88, // *...*.
		0x88, // *...*.
		0x00, // ......
	},
	{ // 66
		0xf0, // ****..
		0x88, // *...*.
		0x88, // *...*.
		0xf0, // ****..
		0x88, // *...*.
		0x88, // *...*.
		0xf0, // ****..
		0x00, // ......
	},
	{ // 67
		0x70, // .***..
		0x88, // *...*.
		0x80, // *.....
		0x80, // *.....
		0x80, // *.....
		0x88, // *...*.
		0x70, // .***..
		0x00, // ......
	},
	{ // 68
		0xf0, // ****..
		0x88, // *...*.
		0x88, // *...*.
		0x88, // *...*.
		0x88, // *...*.
		0x88, // *...*.
		0xf0, // ****..
		0x00, // ......
	},
	{ // 69
		0xf8, // *****.
		0x80, // *.....
		0x80, // *.....
		0xf0, // ****..
		0x80, // *.....
		0x80, // *.....
		0xf8, // *****.
		0x00, // ......
	},
	{ // 70
		0xf8, // *****.
		0x80, // *.....
		0x80, // *.....
		0xf0, // ****..
		0x80, // *.....
		0x80, // *.....
		0x80, // *.....
		0x00, // ......
	},
	{ // 71
		0x70, // .***..
		0x88, // *...*.
		0x80, // *.....
		0x80, // *.....
		0x98, // *..**.
		0x88, // *...*.
		0x78, // .****.
		0x00, // ......
	},
	{ // 72
		0x88, // *...*.
		0x88, // *...*.
		0x88, // *...*.
		0xf8, // *****.
		0x88, // *...*.
		0x88, // *...*.
		0x88, // *...*.
		0x00, // ......
	},
	{ // 73
		0x70, // .***..
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x70, // .***..
		0x00, // ......
	},
	{ // 74
		0x08, // ....*.
		0x08, // ....*.
		0x08, // ....*.
		0x08, // ....*.
		0x08, // ....*.
		0x88, // *...*.
		0x70, // .***..
		0x00, // ......
	},
	{ // 75
		0x88, // *...*.
		0x90, // *..*..
		0xa0, // *.*...
		0xc0, // **....
		0xa0, // *.*...
		0x90, // *..*..
		0x88, // *...*.
		0x00, // ......
	},
	{ // 76
		0x80, // *.....
		0x80, // *.....
		0x80, // *.....
		0x80, // *.....
		0x80, // *.....
		0x80, // *.....
		0xf8, // *****.
		0x00, // ......
	},
	{ // 77
		0x88, // *...*.
		0xd8, // **.**.
		0xa8, // *.*.*.
		0xa8, // *.*.*.
		0x88, // *...*.
		0x88, // *...*.
		0x88, // *...*.
		0x00, // ......
	},
	{ // 78
		0x88, // *...*.
		0x88, // *...*.
		0xc8, // **..*.
		0xa8, // *.*.*.
		0x98, // *..**.
		0x88, // *...*.
		0x88, // *...*.
		0x00, // ......
	},
	{ // 79
		0x70, // .***..
		0x88, // *...*.
		0x88, // *...*.
		0x88, // *...*.
		0x88, // *...*.
		0x88, // *...*.
		0x70, // .***..
		0x00, // ......
	},
	{ // 80
		0xf0, // ****..
		0x88, // *...*.
		0x88, // *...*.
		0xf0, // ****..
		0x80, // *.....
		0x80, // *.....
		0x80, // *.....
		0x00, // ......
	},
	{ // 81
		0x70, // .***..
		0x88, // *...*.
		0x88, // *...*.
		0x88, // *...*.
		0xa8, // *.*.*.
		0x90, // *..*..
		0x68, // .**.*.
		0x00, // ......
	},
	{ // 82
		0xf0, // ****..
		0x88, // *...*.
		0x88, // *...*.
		0xf0, // ****..
		0xa0, // *.*...
		0x90, // *..*..
		0x88, // *...*.
		0x00, // ......
	},
	{ // 83
		0x70, // .***..
		0x88, // *...*.
		0x80, // *.....
		0x70, // .***..
		0x08, // ....*.
		0x88, // *...*.
		0x70, // .***..
		0x00, // ......
	},
	{ // 84
		0xf8, // *****.
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x00, // ......
	},
	{ // 85
		0x88, // *...*.
		0x88, // *...*.
		0x88, // *...*.
		0x88, // *...*.
		0x88, // *...*.
		0x88, // *...*.
		0x70, // .***..
		0x00, // ......
	},
	{ // 86
		0x88, // *...*.
		0x88, // *...*.
		0x88, // *...*.
		0x88, // *...*.
		0x88, // *...*.
		0x50, // .*.*..
		0x20, // ..*...
		0x00, // ......
	},
	{ // 87
		0x88, // *...*.
		0x88, // *...*.
		0x88, // *...*.
		0xa8, // *.*.*.
		0xa8, // *.*.*.
		0xd8, // **.**.
		0x88, // *...*.
		0x00, // ......
	},
	{ // 88
		0x88, // *...*.
		0x88, // *...*.
		0x50, // .*.*..
		0x20, // ..*...
		0x50, // .*.*..
		0x88, // *...*.
		0x88, // *...*.
		0x00, // ......
	},
	{ // 89
		0x88, // *...*.
		0x88, // *...*.
		0x50, // .*.*..
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x00, // ......
	},
	{ // 90
		0xf8, // *****.
		0x08, // ....*.
		0x10, // ...*..
		0x20, // ..*...
		0x40, // .*....
		0x80, // *.....
		0xf8, // *****.
		0x00, // ......
	},
	{ // 91
		0xf8, // *****.
		0xc0, // **....
		0xc0, // **....
		0xc0, // **....
		0xc0, // **....
		0xc0, // **....
		0xf8, // *****.
		0x00, // ......
	},
	{ // 92
		0x00, // ......
		0x80, // *.....
		0x40, // .*....
		0x20, // ..*...
		0x10, // ...*..
		0x08, // ....*.
		0x00, // ......
		0x00, // ......
	},
	{ // 93
		0xf8, // *****.
		0x18, // ...**.
		0x18, // ...**.
		0x18, // ...**.
		0x18, // ...**.
		0x18, // ...**.
		0xf8, // *****.
		0x00, // ......
	},
	{ // 94
		0x00, // ......
		0x00, // ......
		0x20, // ..*...
		0x50, // .*.*..
		0x88, // *...*.
		0x00, // ......
		0x00, // ......
		0x00, // ......
	},
	{ // 95
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0xf8, // *****.
	},
	{ // 96
		0x40, // .*....
		0x20, // ..*...
		0x10, // ...*..
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
	},
	{ // 97
		0x00, // ......
		0x00, // ......
		0x60, // .**...
		0x98, // *..**.
		0x88, // *...*.
		0x98, // *..**.
		0x68, // .**.*.
		0x00, // ......
	},
	{ // 98
		0x40, // .*....
		0x40, // .*....
		0x50, // .*.*..
		0x68, // .**.*.
		0x48, // .*..*.
		0x48, // .*..*.
		0x30, // ..**..
		0x00, // ......
	},
	{ // 99
		0x00, // ......
		0x00, // ......
		0x38, // ..***.
		0x40, // .*....
		0x40, // .*....
		0x40, // .*....
		0x38, // ..***.
		0x00, // ......
	},
	{ // 100
		0x08, // ....*.
		0x08, // ....*.
		0x28, // ..*.*.
		0x58, // .*.**.
		0x48, // .*..*.
		0x48, // .*..*.
		0x30, // ..**..
		0x00, // ......
	},
	{ // 101
		0x00, // ......
		0x00, // ......
		0x30, // ..**..
		0x48, // .*..*.
		0x70, // .***..
		0x40, // .*....
		0x38, // ..***.
		0x00, // ......
	},
	{ // 102
		0x30, // ..**..
		0x28, // ..*.*.
		0x20, // ..*...
		0x70, // .***..
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x00, // ......
	},
	{ // 103
		0x00, // ......
		0x00, // ......
		0x30, // ..**..
		0x48, // .*..*.
		0x48, // .*..*.
		0x38, // ..***.
		0x08, // ....*.
		0x38, // ..***.
	},
	{ // 104
		0x40, // .*....
		0x40, // .*....
		0x50, // .*.*..
		0x68, // .**.*.
		0x48, // .*..*.
		0x48, // .*..*.
		0x48, // .*..*.
		0x00, // ......
	},
	{ // 105
		0x20, // ..*...
		0x00, // ......
		0x60, // .**...
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x70, // .***..
		0x00, // ......
	},
	{ // 106
		0x08, // ....*.
		0x00, // ......
		0x08, // ....*.
		0x08, // ....*.
		0x08, // ....*.
		0x08, // ....*.
		0x48, // .*..*.
		0x70, // .***..
	},
	{ // 107
		0x40, // .*....
		0x40, // .*....
		0x48, // .*..*.
		0x50, // .*.*..
		0x70, // .***..
		0x48, // .*..*.
		0x48, // .*..*.
		0x00, // ......
	},
	{ // 108
		0x60, // .**...
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x70, // .***..
		0x00, // ......
	},
	{ // 109
		0x00, // ......
		0x00, // ......
		0xd0, // **.*..
		0xa8, // *.*.*.
		0xa8, // *.*.*.
		0xa8, // *.*.*.
		0x88, // *...*.
		0x00, // ......
	},
	{ // 110
		0x00, // ......
		0x00, // ......
		0x70, // .***..
		0x48, // .*..*.
		0x48, // .*..*.
		0x48, // .*..*.
		0x48, // .*..*.
		0x00, // ......
	},
	{ // 111
		0x00, // ......
		0x00, // ......
		0x30, // ..**..
		0x48, // .*..*.
		0x48, // .*..*.
		0x48, // .*..*.
		0x30, // ..**..
		0x00, // ......
	},
	{ // 112
		0x00, // ......
		0x00, // ......
		0x30, // ..**..
		0x48, // .*..*.
		0x48, // .*..*.
		0x70, // .***..
		0x40, // .*....
		0x40, // .*....
	},
	{ // 113
		0x00, // ......
		0x00, // ......
		0x38, // ..***.
		0x48, // .*..*.
		0x48, // .*..*.
		0x38, // ..***.
		0x08, // ....*.
		0x08, // ....*.
	},
	{ // 114
		0x00, // ......
		0x00, // ......
		0x58, // .*.**.
		0x68, // .**.*.
		0x40, // .*....
		0x40, // .*....
		0x40, // .*....
		0x00, // ......
	},
	{ // 115
		0x00, // ......
		0x00, // ......
		0x38, // ..***.
		0x40, // .*....
		0x30, // ..**..
		0x08, // ....*.
		0x70, // .***..
		0x00, // ......
	},
	{ // 116
		0x20, // ..*...
		0x20, // ..*...
		0x70, // .***..
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x30, // ..**..
		0x00, // ......
	},
	{ // 117
		0x00, // ......
		0x00, // ......
		0x48, // .*..*.
		0x48, // .*..*.
		0x48, // .*..*.
		0x58, // .*.**.
		0x28, // ..*.*.
		0x00, // ......
	},
	{ // 118
		0x00, // ......
		0x00, // ......
		0x88, // *...*.
		0x88, // *...*.
		0x50, // .*.*..
		0x50, // .*.*..
		0x20, // ..*...
		0x00, // ......
	},
	{ // 119
		0x00, // ......
		0x00, // ......
		0x88, // *...*.
		0xa8, // *.*.*.
		0xa8, // *.*.*.
		0xa8, // *.*.*.
		0xd0, // **.*..
		0x00, // ......
	},
	{ // 120
		0x00, // ......
		0x00, // ......
		0xd8, // **.**.
		0x50, // .*.*..
		0x20, // ..*...
		0x50, // .*.*..
		0xd8, // **.**.
		0x00, // ......
	},
	{ // 121
		0x00, // ......
		0x00, // ......
		0x48, // .*..*.
		0x48, // .*..*.
		0x58, // .*.**.
		0x28, // ..*.*.
		0x08, // ....*.
		0x18, // ...**.
	},
	{ // 122
		0x00, // ......
		0x00, // ......
		0x78, // .****.
		0x08, // ....*.
		0x30, // ..**..
		0x40, // .*....
		0x78, // .****.
		0x00, // ......
	},
	{ // 123
		0x18, // ...**.
		0x60, // .**...
		0x40, // .*....
		0x20, // ..*...
		0x40, // .*....
		0x60, // .**...
		0x18, // ...**.
		0x00, // ......
	},
	{ // 124
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x00, // ......
		0x20, // ..*...
		0x20, // ..*...
		0x20, // ..*...
		0x00, // ......
	},
	{ // 125
		0xc0, // **....
		0x30, // ..**..
		0x10, // ...*..
		0x20, // ..*...
		0x10, // ...*..
		0x30, // ..**..
		0xc0, // **....
		0x00, // ......
	},
	{ // 126
		0x40, // .*....
		0xa8, // *.*.*.
		0x10, // ...*..
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
		0x00, // ......
	},
	{ // 127
		0xa8, // *.*.*.
		0x50, // .*.*..
		0xa8, // *.*.*.
		0x50, // .*.*..
		0xa8, // *.*.*.
		0x50, // .*.*..
		0xa8, // *.*.*.
		0x00, // ......
	},
	{ // 128
		0x00, // ......
		0x00, // ......
		0x30, // ..**..
		0x78, // .****.
		0x78, // .****.
		0x30, // ..**..
		0x00, // ......
		0x00, // ......
	},
};

static const int LowCharacterCode		= 0;
static const int HighCharacterCode		= sizeof(font)/sizeof(font[0]);
static const int CharacterCodeCount		= HighCharacterCode-LowCharacterCode;

static void blitToTexture(xBitmapData& dest)
{
	// TODO : Turn this into a single-byte alpha mask

	int2 texdim = { 6 * HighCharacterCode, 8 };
	dest.buffer.Resize(texdim.x * texdim.y * 4);
	dest.width	= texdim.x;
	dest.height = texdim.y;

	u32* destPtr = (u32*)dest.buffer.GetPtr();
	for (int ch=0; ch<HighCharacterCode; ++ch) {
		for (int y=0; y<8; ++y) {
			for (int bit=0; bit<6; ++bit) {
				u32 isCovered = ((font[ch][y] >> (7-bit)) & 1) ? 0xff : 0;
				destPtr[(ch*6) + (y * texdim.x) + bit] = (isCovered <<  0) | (isCovered <<  8) | (isCovered << 16) | (isCovered << 24);
			}
		}
	}
}

static GPU_TextureResource2D	tex_6x8;
static GPU_ShaderVS				s_ShaderVS_DbgFont;
static GPU_ShaderFS				s_ShaderFS_DbgFont;
static GPU_VertexBuffer			s_mesh_anychar;
static GPU_VertexBuffer			s_mesh_worldViewTileID;
static GPU_ConstantBuffer		s_cnstbuf_Projection;
static GPU_ConstantBuffer		s_cnstbuf_DbgFontSheet;
static GPU_IndexBuffer			s_idx_UniformQuad;

static GPU_ViewCameraConsts		m_ViewConsts;

typedef std::vector<DbgFontDrawItem> DbgFontDrawItemContainer;

DbgFontDrawItemContainer		g_dbgFontDrawList;

DbgFontSheet					g_ConsoleSheet;
DbgFontSheet					g_DbgFontOverlay;
bool							s_canRender = false;

void DbgFontSheet::AllocSheet(int2 sizeInPix)
{
	size.x		= sizeInPix.x	/ font.size.x;
	size.y		= sizeInPix.y	/ font.size.y;
	charmap		= (DbgChar*) xRealloc(charmap,  size.y * size.x * sizeof(DbgChar));
	colormap	= (DbgColor*)xRealloc(colormap, size.y * size.x * sizeof(DbgColor));

	dx11_CreateDynamicVertexBuffer(gpu.mesh_charmap, size.y * size.x * sizeof(DbgChar ));
	dx11_CreateDynamicVertexBuffer(gpu.mesh_rgbamap, size.y * size.x * sizeof(DbgColor));
}

template< typename T >
void xMemSetObjs(T* dest, const T& data, int sizeInInstances)
{
	for (int i=0; i<sizeInInstances; ++i) {
		dest[i] = data;
	}
}

void DbgFontSheet::SceneLogic()
{
	xMemSetObjs (charmap,   { 0 },							size.y * size.x);
	xMemSetObjs	(colormap, {{ 1.0f, 0.5f, 0.5f, 0.5f }},	size.y * size.x);
}

void DbgFontSheet::Write(int x, int y, const xString& msg)
{
	int pos = (y * size.x) + x;
	for (int i=0; i<msg.GetLength(); ++i) {
		g_DbgFontOverlay.charmap[pos+i] = (u8)msg[i];
	}
}


template< typename T >
void table_get_xy(T& dest, LuaTableScope& table)
{
	auto xs = table.get_s32("x");
	auto ys = table.get_s32("y");

	if (xs.isnil()) { xs = table.get_s32(1); }
	if (ys.isnil()) { ys = table.get_s32(2); }

	if (xs.isnil() || ys.isnil()) {
		bug_qa("Invalid table layout for expected Int2 parameter.");
		// Todo: expose a ThrowError that pushes message onto Lua stack and invokes longjmp.
		// Pre-req: must have a single global lua state for all scene magic.
		//ThrowError
	}

	dest.x = xs;
	dest.y = ys;
}

template< typename T >
bool table_get_xy(T& dest, LuaTableScope& table, const char* subtable)
{
	bug_on(!subtable);
	if (auto& subtab = table.get_table(subtable)) {
		table_get_xy(dest, subtab);
		return true;
	}
	return false;
}

static GPU_InputDesc InputLayout_DbgFont;

void DbgFont_MakeVertexLayout()
{
	xMemZero(InputLayout_DbgFont);
	InputLayout_DbgFont.AddVertexSlot( {
		{ "POSITION", GPU_ResourceFmt_R32G32_FLOAT	},
		{ "TEXCOORD", GPU_ResourceFmt_R32G32_FLOAT	}
	});

	InputLayout_DbgFont.AddInstanceSlot( {
		{ "TileID", GPU_ResourceFmt_R32_UINT }
	});

	InputLayout_DbgFont.AddInstanceSlot( {
		{ "COLOR",  GPU_ResourceFmt_R32G32B32A32_FLOAT }
	});
}

void DbgFont_LoadInit()
{
	auto& script = g_scriptEnv;

	s_canRender = 0;

	// TODO:
	//  What might be nice here is to build all the known tables and populate them with defaults during
	//  a lua-environment-init step.  Members can be given metadata comments, if desired.  Once populated,
	//  the tables can be displayed using Lua Inspect module (should be included in the repo), eg:
    //
	//     $ print(inspect(DbgConsole))
	//
	//  ... which neatly displays all members supported by the game engine, including metadata comments,
	//  if provided.
	//
	//  Likewise, the console itself can support TAB autocompletion based on table inspection.  Would be
	//  super-cool, right?
	//
	// Drawbacks:
	//   * inspect(table) is not available from external editors.
	//       Workaround: full inspection dump can be written as plaintext or JSON when process starts, and
	//       can be used at a minimum provide a copy/paste template for setting up assignment lists for data-
	//       driven structures.  Advanced integration could include autocomplete -- minimal usefulness for
	//       data-driven things, but higher usefulness for function-driven things.
	//

	float2 edgeOffset		 = float2 { 4, 4 };
	float2 backbuffer_size	 = (g_backbuffer_size_pix - edgeOffset);

	// Provide a fixed-size font grid that scales "roughly" to match the backbuffer resolution.
	// use floorf to ensure the scalar is even-numbered, to avoid uglified font syndrome.
	// (note: in theory above 3x scale it's probably ok to be fractional, as it won't really look bad anyway...)

	auto scalarxy = floorf(g_backbuffer_size_pix / float2 { 640, 360 });
	auto scalar = std::max(scalarxy.x, scalarxy.y);

	edgeOffset		 /= scalar;
	backbuffer_size  /= scalar;

	g_ConsoleSheet.font.size		= { 6, 8 };
	g_DbgFontOverlay.font.size		= { 6, 8 };
	auto consoleSizeInPix			= (int2)floorf(g_backbuffer_size_pix / 2	);
	auto overlaySizeInPix			= (int2)floorf(backbuffer_size);

	lua_string consoleShaderFile;
	lua_string consoleShaderEntryVS;
	lua_string consoleShaderEntryFS;

	script.LoadModule("scripts/DbgConsole.lua");

	if (script.HasError()) {
		bug("Unhandled error in DbgFont?");
	}

	if (auto& dbgtable = script.glob_open_table("DbgConsole"))
	{
		table_get_xy(g_ConsoleSheet.font.size,	dbgtable, "CharSize");
		table_get_xy(consoleSizeInPix,			dbgtable, "SheetSize");

		if (auto& shadertab = dbgtable.get_table("Shader")) {
			consoleShaderFile		= shadertab.get_string("Filename");
			consoleShaderEntryVS	= shadertab.get_string("VS");
			consoleShaderEntryFS	= shadertab.get_string("FS");

			if (consoleShaderFile.isnil()) {
				if (1)								{ consoleShaderFile		= shadertab.get_string(1); }
				if (consoleShaderEntryVS.isnil())	{ consoleShaderEntryVS	= shadertab.get_string(2); }
				if (consoleShaderEntryFS.isnil())	{ consoleShaderEntryFS	= shadertab.get_string(3); }
			}
		}
	}


	if (1) {
		xBitmapData  pngtex;
		blitToTexture(pngtex);
		dx11_CreateTexture2D(tex_6x8, pngtex.buffer.GetPtr(), pngtex.width, pngtex.height, GPU_ResourceFmt_R8G8B8A8_UNORM);
	}

	g_ConsoleSheet.font.texture		= &tex_6x8;
	g_ConsoleSheet.AllocSheet(consoleSizeInPix);

	g_DbgFontOverlay.font.texture	= &tex_6x8;
	g_DbgFontOverlay.AllocSheet(overlaySizeInPix);

	dx11_LoadShaderVS(s_ShaderVS_DbgFont, consoleShaderFile, consoleShaderEntryVS);
	dx11_LoadShaderFS(s_ShaderFS_DbgFont, consoleShaderFile, consoleShaderEntryFS);

	dx11_CreateConstantBuffer(s_cnstbuf_Projection,		sizeof(m_ViewConsts));
	dx11_CreateConstantBuffer(s_cnstbuf_DbgFontSheet,	sizeof(g_DbgFontOverlay.gpu.consts));
	dx11_CreateIndexBuffer(s_idx_UniformQuad, g_ind_UniformQuad, sizeof(g_ind_UniformQuad));

	dx11_CreateStaticMesh(s_mesh_anychar,	g_mesh_UniformQuad,	sizeof(g_mesh_UniformQuad[0]),	bulkof(g_mesh_UniformQuad));

	u128	m_Eye;
	u128	m_At;
	u128	m_Up;			// X is angle.  Y is just +/- (orientation)? Z is unused?

	//m_Eye	= XMVectorSet( overlaySizeInPix.x/2, overlaySizeInPix.y/2, -6.0f, 0.0f );
	//m_At	= XMVectorSet( overlaySizeInPix.x/2, overlaySizeInPix.y/2,  0.0f, 0.0f );

	m_Eye	= XMVectorSet( 0.0f, 0.0f, -6.0f, 0.0f );
	m_At	= XMVectorSet( 0.0f, 0.0f,  0.0f, 0.0f );
	m_Up	= XMVectorSet( 0.0f, 1.0f,  0.0f, 0.0f );

	m_ViewConsts.View		= XMMatrixLookAtLH(m_Eye, m_At, m_Up);
	m_ViewConsts.Projection = XMMatrixOrthographicLH(overlaySizeInPix.x, overlaySizeInPix.y, 0.0001f, 1000.0f);
	m_ViewConsts.Projection = XMMatrixOrthographicOffCenterLH(-edgeOffset.x, backbuffer_size.x, backbuffer_size.y, -edgeOffset.y, 0.0001f, 1000.0f);

	s_canRender = 1;
}

void DbgFont_SceneBegin()
{
	g_DbgFontOverlay	.SceneLogic();
	g_ConsoleSheet		.SceneLogic();
}

void DbgFont_SceneRender()
{
	if (!s_canRender) return;

	// Update dynamic vertex buffers.

	int overlayMeshSize = g_DbgFontOverlay.size.x * g_DbgFontOverlay.size.y;

	g_DbgFontOverlay.Write(0,0, "TESTING");

	dx11_UploadDynamicBufferData(g_DbgFontOverlay.gpu.mesh_charmap, g_DbgFontOverlay.charmap,  overlayMeshSize * sizeof(DbgChar ));
	dx11_UploadDynamicBufferData(g_DbgFontOverlay.gpu.mesh_rgbamap, g_DbgFontOverlay.colormap, overlayMeshSize * sizeof(DbgColor));

	g_DbgFontOverlay.gpu.consts.SrcTexTileSizeUV	= vFloat2(1.0f / CharacterCodeCount, 1.0f);
	g_DbgFontOverlay.gpu.consts.SrcTexSizeInTiles	= vInt2(CharacterCodeCount,1);
	g_DbgFontOverlay.gpu.consts.CharMapSize.x		= g_DbgFontOverlay.size.x;
	g_DbgFontOverlay.gpu.consts.CharMapSize.y		= g_DbgFontOverlay.size.y;
	g_DbgFontOverlay.gpu.consts.TileSize			= g_DbgFontOverlay.font.size;
	//g_DbgFontOverlay.gpu.consts.ProjectionXY		= vFloat2(0.0f, 0.0f);
	//g_DbgFontOverlay.gpu.consts.ProjectionScale		= vFloat2(1.0f, 1.0f);

	GPU_ViewCameraConsts	viewConsts;
	viewConsts.View			= XMMatrixTranspose(m_ViewConsts.View);
	viewConsts.Projection	= XMMatrixTranspose(m_ViewConsts.Projection);

	dx11_UpdateConstantBuffer(s_cnstbuf_Projection,		&viewConsts);
	dx11_UpdateConstantBuffer(s_cnstbuf_DbgFontSheet,	&g_DbgFontOverlay.gpu.consts);

	// Render!

	DbgFont_MakeVertexLayout();
	dx11_SetInputLayout(InputLayout_DbgFont);

	dx11_BindShaderVS(s_ShaderVS_DbgFont);
	dx11_BindShaderFS(s_ShaderFS_DbgFont);

	dx11_BindShaderResource(tex_6x8, 0);

	dx11_SetVertexBuffer(s_mesh_anychar,					0, sizeof(g_mesh_UniformQuad[0]), 0);
	dx11_SetVertexBuffer(g_DbgFontOverlay.gpu.mesh_charmap,	1, sizeof(DbgChar),  0);
	dx11_SetVertexBuffer(g_DbgFontOverlay.gpu.mesh_rgbamap,	2, sizeof(DbgColor), 0);

	//dx11_SetVertexBuffer(g_mesh_worldViewColor, 2, sizeof(g_ViewUV[0]), 0);

	dx11_BindConstantBuffer(s_cnstbuf_Projection,	0);
	dx11_BindConstantBuffer(s_cnstbuf_DbgFontSheet, 1);
	dx11_SetIndexBuffer(s_idx_UniformQuad, 16, 0);
	dx11_SetPrimType(GPU_PRIM_TRIANGLELIST);
	dx11_DrawIndexedInstanced(6, overlayMeshSize, 0, 0, 0);

}
