
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

void WorldMap_Procgen()
{
	g_WorldMap	= (TerrainMapItem*)	xRealloc(g_WorldMap, WorldSizeX    * WorldSizeY    * sizeof(TerrainMapItem));

	// Fill map with boring grass.

	for (int y=0; y<WorldSizeY; ++y) {
		for (int x=0; x<WorldSizeX; ++x) {
			g_WorldMap[(y * WorldSizeX) + x].tilesetId = 11;
		}
	}

	// Write a border around the entire map for now.

	for (int x=0; x<WorldSizeX; ++x) {
		g_WorldMap[((     0	     ) * WorldSizeX) + x].tilesetId = 21;
		g_WorldMap[((     1	     ) * WorldSizeX) + x].tilesetId = 22;
		g_WorldMap[((WorldSizeY-1) * WorldSizeX) + x].tilesetId = 22;
		g_WorldMap[((WorldSizeY-2) * WorldSizeX) + x].tilesetId = 21;
	}

	for (int y=0; y<WorldSizeY; ++y) {
		g_WorldMap[(y * WorldSizeX) + 0				].tilesetId = 21;
		g_WorldMap[(y * WorldSizeX) + 1				].tilesetId = 21;
		g_WorldMap[(y * WorldSizeX) + (WorldSizeX-2)].tilesetId = 21;
		g_WorldMap[(y * WorldSizeX) + (WorldSizeX-1)].tilesetId = 21;
	}
}

void OpenWorldEnviron::InitScene()
{
	if (1) {
		xBitmapData  pngtex;
		png_LoadFromFile(pngtex, ".\\rpg_maker_vx__modernrtp_tilea2_by_painhurt-d3f7rwg.png");

		// cut sets out of the source and paste them into a properly-formed TextureAtlas.

		// Assume pngtex is rpgmaker layout for now.
		// Complete Sets are 64 px wide and 96 px tall (32px set + 64px set)
		// Within those are several subsets... there's a text file describing them, search for rpgmaker.

		int2 setSize	= {64, 96};
		int2 tileSize	= {16, 16};
		int2 setToGrab	= {5, 2};
		auto sizeInSets = pngtex.size / setSize;

		TextureAtlas atlas;
		xBitmapData  tile;

		atlas.Init(tileSize);

		auto topLeft = setToGrab * setSize;
		topLeft.y += 32;	// grab the area set.

		x_png_enc pngenc;

		for (int y=0; y<4; ++y, topLeft.y += 16) {
			auto tl = topLeft;
			for (int x=0; x<4; ++x, tl.x += 16) {
				imgtool::AddTileToAtlas(atlas, pngtex, tl);
			}
		}

		atlas.Solidify();
		pngenc.WriteImage(atlas);

		pragma_todo("Create a global temp dir mount and dump things to subdirs in there...");
		xCreateDirectory("..\\tempout\\");
		pngenc.SaveImage(xFmtStr("..\\tempout\\atlas.png"));

		g_GroundLayer.SetSourceTexture(atlas);
	}

	WorldMap_Procgen();
	g_GroundLayer.PopulateUVs(g_WorldMap, {0,0});

	fmod_CreateMusic(s_music_world, "..\\unity\\Assets\\Audio\\Music\\ff2over.s3m");
}

void OpenWorldEnviron::Tick()
{
	g_GroundLayer.CenterViewOn({ g_ViewCamera.m_Eye.x, g_ViewCamera.m_Eye.y });
	g_GroundLayer.PopulateUVs(g_WorldMap);

	fmod_Play(s_music_world);
	if (ImGui::SliderFloat("BGM Volume", &s_bgm_volume, 0, 1.0f)) {
		fmod_SetVolume(s_music_world, s_bgm_volume);
	}
}
