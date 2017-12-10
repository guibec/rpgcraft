
#include "PCH-rpgcraft.h"
#include "TileMapLayer.h"

#include "x-png-decode.h"
#include "x-png-encode.h"
#include "fmod-ifc.h"
#include "imgtools.h"

#include "imgui.h"

TerrainMapItem*		g_WorldMap		= nullptr;
TerrainMapItem*		g_wm_SubLayer	= nullptr;

static FmodMusic	s_music_world;
static float		s_bgm_volume =	1.0f;

void WorldMap_Procgen()
{
	g_WorldMap		= (TerrainMapItem*)	xRealloc(g_WorldMap,	WorldSizeX    * WorldSizeY    * sizeof(TerrainMapItem));
	g_wm_SubLayer	= (TerrainMapItem*)	xRealloc(g_wm_SubLayer, WorldSizeX    * WorldSizeY    * sizeof(TerrainMapItem));

	// Fill map with boring grass.

	for (int y=0; y<WorldSizeY; ++y) {
		for (int x=0; x<WorldSizeX; ++x) {
			g_WorldMap		[(y * WorldSizeX) + x].tilesetId = 14;
			g_wm_SubLayer	[(y * WorldSizeX) + x].tilesetId = 31;
		}
	}

	// Write a border around the entire map for now.

	for (int x=0; x<WorldSizeX; ++x) {
		g_WorldMap[((     0	     ) * WorldSizeX) + x].tilesetId = 1;
		//g_WorldMap[((     1	     ) * WorldSizeX) + x].tilesetId = 2;
		//g_WorldMap[((WorldSizeY-1) * WorldSizeX) + x].tilesetId = 2;
		g_WorldMap[((WorldSizeY-2) * WorldSizeX) + x].tilesetId = 1;
	}

	for (int y=0; y<WorldSizeY; ++y) {
		g_WorldMap[(y * WorldSizeX) + 0				].tilesetId = 4;
		//g_WorldMap[(y * WorldSizeX) + 1				].tilesetId = 4;
		//g_WorldMap[(y * WorldSizeX) + (WorldSizeX-2)].tilesetId = 4;
		g_WorldMap[(y * WorldSizeX) + (WorldSizeX-1)].tilesetId = 4;
	}
}

namespace TerrainSetStandardTile {
	enum enum_t
	{
		Solid = 0,

		ObtuseCorners,
		Obtuse_HiL		= ObtuseCorners,
		Obtuse_HiR,
		Obtuse_LoL,
		Obtuse_LoR,

		AcuteCorners,
		Acute_HiL		= AcuteCorners,
		Acute_HiR,
		Acute_LoL,
		Acute_LoR,

		Spans,
		Span_HorizHi	= Spans,
		Span_HorizLo,
		Span_VertL,
		Span_VertR,

		NUM_STD_TILES
	};

	using TerrainSetStandardTile_t = TerrainSetStandardTile::enum_t;
	static const int2 RipSrcTilePos[NUM_STD_TILES];
}

pragma_todo("FIXME: Fill this in and apply it to GrabTerrainSet2()");
static const int2 GrabCoords[TerrainSetStandardTile::NUM_STD_TILES] = {
	{ }, // Solid,
	{ }, // Obtuse_HiL
	{ }, // Obtuse_HiR
	{ }, // Obtuse_LoL
	{ }, // Obtuse_LoR
	{ }, // Acute_HiL,
	{ }, // Acute_HiR
	{ }, // Acute_LoL
	{ }, // Acute_LoR
	{ }, // Span_HorizHi
	{ }, // Span_HorizLo
	{ }, // Span_VertL
	{ }, // Span_VertR
};


static void GrabTerrainSet2(TextureAtlas& atlas, const xBitmapData& pngtex, const int2& setSize, const int2& setToGrab)
{
	auto topLeft = setToGrab * setSize;
	const auto& tileSize = atlas.m_tileSizePix;

	topLeft.y += 64;	// grab the area set.

	x_png_enc pngenc;

	for (int y=0; y<3; ++y, topLeft.y += tileSize.y) {
		auto tl = topLeft;
		for (int x=0; x<3; ++x, tl.x += tileSize.x) {
			imgtool::AddTileToAtlas(atlas, pngtex, tl);
		}
	}
}

void OpenWorldEnviron::InitScene()
{
	if (0) {
		xBitmapData  pngtex;
		png_LoadFromFile(pngtex, ".\\rpg_maker_vx__modernrtp_tilea2_by_painhurt-d3f7rwg.png");

		// cut sets out of the source and paste them into a properly-formed TextureAtlas.

		// Assume pngtex is rpgmaker layout for now.
		// Complete Sets are 64 px wide and 96 px tall (32px set + 64px set)
		// Within those are several subsets... there's a text file describing them, search for rpgmaker.

		int2 setSize	= {64, 96};
		int2 tileSize	= {16, 16};
		auto sizeInSets = pngtex.size / setSize;

		TextureAtlas atlas;

		atlas.Init(tileSize);

		int2 setToGrab	= {5, 2};
		auto topLeft = setToGrab * setSize;
		topLeft.y += 32;	// grab the area set.


		for (int y=0; y<4; ++y, topLeft.y += 16) {
			auto tl = topLeft;
			for (int x=0; x<4; ++x, tl.x += 16) {
				imgtool::AddTileToAtlas(atlas, pngtex, tl);
			}
		}

		atlas.Solidify();

		x_png_enc pngenc;
		pngenc.WriteImage(atlas);

		pragma_todo("Create a global temp dir mount and dump things to subdirs in there...");
		xCreateDirectory("..\\tempout\\");
		pngenc.SaveImage(xFmtStr("..\\tempout\\atlas.png"));

		g_GroundLayer.SetSourceTexture(atlas);
	}

	if (1) {
		xBitmapData  pngtex;
		png_LoadFromFile(pngtex, ".\\sheets\\tiles\\terrain_2.png");

		// cut sets out of the source and paste them into a properly-formed TextureAtlas.

		// terrain2 is designed a bit differently than the painhurt set:
		//   * Each tile is 32x32 pix
		//   * Each terrain set is 96x192 pixels
		//   * Sets are subdivided into four smaller sets:
		//        32x64  -- highlight decals for being placed on top of other terrain types
		//        64x64  -- obtuse turns
		//        96x96  -- acute turns and filler
		//        96x32  -- four fill tiles
		//   * In some cases the highlight decal is a single 32x64 tile, rather than two independent tiles,
		//     and might even be some special decoration unrelated to the tile set.

		int2 setSize	= {96, 192};
		int2 tileSize	= {32, 32};
		auto sizeInSets = pngtex.size / setSize;

		TextureAtlas atlas;
		atlas.Init(tileSize);

		GrabTerrainSet2(atlas, pngtex, setSize, {0, 1});
		GrabTerrainSet2(atlas, pngtex, setSize, {5, 0});
		atlas.Solidify();

		x_png_enc pngenc;
		pngenc.WriteImage(atlas);

		xCreateDirectory("..\\tempout\\");
		pngenc.SaveImage(xFmtStr("..\\tempout\\atlas2.png"));

		g_GroundLayer	.SetSourceTexture(atlas);
		g_GroundSubLayer.SetSourceTexture(atlas);
	}

	WorldMap_Procgen();
	g_GroundLayer.PopulateUVs(g_WorldMap, {0,0});
	g_GroundSubLayer.PopulateUVs(g_wm_SubLayer, {0,0});

	fmod_CreateMusic(s_music_world, "..\\unity\\Assets\\Audio\\Music\\ff2over.s3m");
}

void OpenWorldEnviron::Tick()
{
	g_GroundLayer.CenterViewOn({ g_ViewCamera.m_Eye.x, g_ViewCamera.m_Eye.y });
	g_GroundLayer.PopulateUVs(g_WorldMap);

	g_GroundSubLayer.CenterViewOn({ g_ViewCamera.m_Eye.x, g_ViewCamera.m_Eye.y });
	g_GroundSubLayer.PopulateUVs(g_wm_SubLayer);

	fmod_Play(s_music_world);
	if (ImGui::SliderFloat("BGM Volume", &s_bgm_volume, 0, 1.0f)) {
		fmod_SetVolume(s_music_world, s_bgm_volume);
	}
}
