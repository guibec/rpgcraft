
#include "PCH-rpgcraft.h"
#include "TileMapLayer.h"

#include "x-png-decode.h"
#include "x-png-encode.h"
#include "fmod-ifc.h"
#include "imgtools.h"

#include "imgui.h"

TerrainMapItem*		g_WorldMap		= nullptr;

static FmodMusic	s_music_world;
static float		s_bgm_volume =	1.0f;

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

namespace StdTileOffset
{
	using TerrainSetStandardTile::NUM_STD_TILES;

	static const int Water			= 0 * NUM_STD_TILES;
	static const int Sandy			= 1 * NUM_STD_TILES;
	static const int Grassy			= 2 * NUM_STD_TILES;
}


void WorldMap_Procgen()
{
	g_WorldMap		= (TerrainMapItem*)	xRealloc(g_WorldMap,	WorldSizeX    * WorldSizeY    * sizeof(TerrainMapItem));

	// Fill map with boring grass.  or sand.

	for (int y=0; y<WorldSizeY; ++y) {
		for (int x=0; x<WorldSizeX; ++x) {
			g_WorldMap		[(y * WorldSizeX) + x].underlay = StdTileOffset::Water;
			g_WorldMap		[(y * WorldSizeX) + x].overlay  = StdTileOffset::Sandy;
		}
	}
}

static const int2 T2_GrabCoords[TerrainSetStandardTile::NUM_STD_TILES] = {
	{ 1, 3 },	// Solid,
	{ 1, 0 },	// Obtuse_HiL
	{ 2, 0 },	// Obtuse_HiR
	{ 1, 1 },	// Obtuse_LoL
	{ 2, 1 },	// Obtuse_LoR
	{ 0, 2 },	// Acute_HiL,
	{ 2, 2 },	// Acute_HiR
	{ 0, 4 },	// Acute_LoL
	{ 2, 4 },	// Acute_LoR
	{ 1, 2 },	// Span_HorizHi
	{ 1, 2 },	// Span_HorizLo
	{ 0, 3 },	// Span_VertL
	{ 2, 3 },	// Span_VertR
};


static void GrabTerrainSet2(TextureAtlas& atlas, const xBitmapData& pngtex, const int2& setSize, const int2& setToGrab)
{
	auto topLeft = setToGrab * setSize;
	const auto& tileSize = atlas.m_tileSizePix;

	//topLeft.y += 64;	// grabbing from the area set.

	x_png_enc pngenc;
	for(const auto& coord : T2_GrabCoords) {
		auto tl = topLeft + (coord * tileSize);
		imgtool::AddTileToAtlas(atlas, pngtex, tl);
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

		GrabTerrainSet2(atlas, pngtex, setSize, {0, 2});
		GrabTerrainSet2(atlas, pngtex, setSize, {6, 0});
		atlas.Solidify();

		x_png_enc pngenc;
		pngenc.WriteImage(atlas);

		xCreateDirectory("..\\tempout\\");
		pngenc.SaveImage(xFmtStr("..\\tempout\\atlas2.png"));

		g_GroundLayer	.SetSourceTexture(atlas);
		g_GroundSubLayer.SetSourceTexture(atlas);
	}

	WorldMap_Procgen();
	g_GroundSubLayer.PopulateUVs(g_WorldMap, 2, 0, {0,0});
	g_GroundLayer	.PopulateUVs(g_WorldMap, 2, 1, {0,0});

	fmod_CreateMusic(s_music_world, "..\\unity\\Assets\\Audio\\Music\\ff2over.s3m");
}

void OpenWorldEnviron::Tick()
{
	g_GroundLayer.CenterViewOn({ g_ViewCamera.m_Eye.x, g_ViewCamera.m_Eye.y });
	g_GroundLayer.PopulateUVs(g_WorldMap, 2, 0);

	g_GroundSubLayer.CenterViewOn({ g_ViewCamera.m_Eye.x, g_ViewCamera.m_Eye.y });
	g_GroundSubLayer.PopulateUVs(g_WorldMap, 2, 1);

	fmod_Play(s_music_world);
	if (ImGui::SliderFloat("BGM Volume", &s_bgm_volume, 0, 1.0f)) {
		fmod_SetVolume(s_music_world, s_bgm_volume);
	}
}
