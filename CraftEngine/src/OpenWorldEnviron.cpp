
#include "PCH-rpgcraft.h"
#include "TileMapLayer.h"

#include "x-png-decode.h"
#include "fmod-ifc.h"

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

		// Assume pngtex is rpgmaker layout for now.

		int setx = pngtex.size.x	/ 64;
		int sety = pngtex.size.y	/ (64 + 32);
		g_GroundLayer.SetSourceTexture(pngtex, { setx, sety });
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
