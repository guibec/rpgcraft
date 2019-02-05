
#include "PCH-rpgcraft.h"
#include "TileMapLayer.h"

#include "x-png-decode.h"
#include "x-png-encode.h"

#include "appConfig.h"
#include "fmod-ifc.h"
#include "imgtools.h"

#include "dev-ui/ui-assets.h"
#include "imgui.h"

TileMapItem*        g_TileMap       = nullptr;
TerrainMapItem*     g_TerrainMap    = nullptr;
AudioSettings       g_settings_audio;

static FmodMusic    s_music_world;

static const int TerrainTileConstruct_Count = 1;

namespace StdTileOffset
{
    static const int Empty          = 0;
    static const int Water          = 1 + (0 * TerrainTileConstruct_Count);
    static const int Sandy          = 1 + (1 * TerrainTileConstruct_Count);
    static const int Grassy         = 1 + (2 * TerrainTileConstruct_Count);
}

static const int s_StdTileOffset[] = {
    StdTileOffset::Empty,
    StdTileOffset::Water,
    StdTileOffset::Sandy,
    StdTileOffset::Grassy
};

int getStdTileImageId(TerrainClass terrain) {
    return s_StdTileOffset[int(terrain)];
}

template< typename T, typename T2 >
inline __ai bool xClampCheck(const T& src, const T2& boundsXY) {
    return (src < boundsXY.x) || (src > boundsXY.y);
}

union TileMatchBits {
    struct {
        u8      N   : 1;
        u8      E   : 1;
        u8      S   : 1;
        u8      W   : 1;

        u8      NW  : 1;
        u8      NE  : 1;
        u8      SE  : 1;
        u8      SW  : 1;
    };

    u8      b;

    bool    isAll   ()          const { return b == 0xff; }
    bool    isNone  ()          const { return b == 0; }
};

// Parameters:
//     tileDecorType - for defining variety in apperance, can be unsed for now until such time we want to "pretty things up"
void PlaceTileWithRules(TerrainClass terrain, int tileDecorType, int2 pos)
{
    auto  thisIdx       = (pos.y * WorldSizeX) + pos.x;
    int4  edgesIdx      = {
        ((pos.y + -1) * WorldSizeX) + pos.x +  0,
        ((pos.y +  0) * WorldSizeX) + pos.x +  1,
        ((pos.y +  1) * WorldSizeX) + pos.x +  0,
        ((pos.y +  0) * WorldSizeX) + pos.x + -1,
    };

    int4  cornersIdx    = {
        edgesIdx.x - 1,
        edgesIdx.x + 1,
        edgesIdx.z - 1,
        edgesIdx.z + 1,
    };

    auto& thisTile      = g_TileMap[thisIdx];
    auto& thisTerrain   = g_TerrainMap[thisIdx];

    //  TODO details:
    //   * This probably requires modifying neighboring tiles as well.
    //   * bounds checking on edgesIdx and cornersIdx is needed!  (for now can assume out-of-bounds edges are "water")

    int2 worldBounds = { 0, (WorldSizeX * WorldSizeY) - 1 };

    TileMapItem     outofboundsTile;
    TerrainMapItem  outofboundsTerrain;

    outofboundsTile.tile_below  = StdTileOffset::Water;
    outofboundsTile.tile_above  = StdTileOffset::Empty;
    outofboundsTerrain.class_below = TerrainClass::Water;
    outofboundsTerrain.class_above = TerrainClass::Empty;

    // Gloriously inefficient and entirely effective world bounds checking.
    // Anything out of bounds becomes a water tile for matching purposes.
    // NESW - north, east, south, west.

    auto& edgeN     = xClampCheck(edgesIdx.x,   worldBounds) ? outofboundsTerrain  : g_TerrainMap[edgesIdx.x];
    auto& edgeE     = xClampCheck(edgesIdx.y,   worldBounds) ? outofboundsTerrain  : g_TerrainMap[edgesIdx.y];
    auto& edgeS     = xClampCheck(edgesIdx.z,   worldBounds) ? outofboundsTerrain  : g_TerrainMap[edgesIdx.z];
    auto& edgeW     = xClampCheck(edgesIdx.w,   worldBounds) ? outofboundsTerrain  : g_TerrainMap[edgesIdx.w];

    auto& cornerNW  = xClampCheck(cornersIdx.x, worldBounds) ? outofboundsTerrain  : g_TerrainMap[cornersIdx.x];
    auto& cornerNE  = xClampCheck(cornersIdx.y, worldBounds) ? outofboundsTerrain  : g_TerrainMap[cornersIdx.y];
    auto& cornerSE  = xClampCheck(cornersIdx.w, worldBounds) ? outofboundsTerrain  : g_TerrainMap[cornersIdx.z];
    auto& cornerSW  = xClampCheck(cornersIdx.z, worldBounds) ? outofboundsTerrain  : g_TerrainMap[cornersIdx.w];

    // edge matching algo is probably going to _pretty_ complicated.  Just sayin'.  --jstine

    TerrainClass            a_class     = TerrainClass::Empty;

    TileMatchBits   matched;

    matched.N  = (edgeN.class_below == terrain) || (edgeN.class_above == terrain);
    matched.E  = (edgeE.class_below == terrain) || (edgeE.class_above == terrain);
    matched.S  = (edgeS.class_below == terrain) || (edgeS.class_above == terrain);
    matched.W  = (edgeW.class_below == terrain) || (edgeW.class_above == terrain);

    matched.NW = (cornerNW.class_below == terrain) || (cornerNW.class_above == terrain);
    matched.NE = (cornerNE.class_below == terrain) || (cornerNE.class_above == terrain);
    matched.SE = (cornerSE.class_below == terrain) || (cornerSE.class_above == terrain);
    matched.SW = (cornerSW.class_below == terrain) || (cornerSW.class_above == terrain);

    if (matched.isNone()) {
        thisTile.tile_above = getStdTileImageId(terrain);
    }

    if (!matched.isAll()) {
        // some unmatched neighboring tiles.  Current and nearby tiles will need to be given class_above
        // assignment in order to maintain visual consistency...


    }
}

void WorldMap_Procgen()
{
    g_TileMap      = (TileMapItem*)    xRealloc(g_TileMap,     WorldSizeX    * WorldSizeY    * sizeof(TileMapItem));
    g_TerrainMap   = (TerrainMapItem*) xRealloc(g_TerrainMap,  WorldSizeX    * WorldSizeY    * sizeof(TerrainMapItem));

    memset(g_TileMap, 0, WorldSizeX    * WorldSizeY    * sizeof(TileMapItem));

    // Fill map with boring grass.  or sand.

    for (int y=0; y<WorldSizeY; ++y) {
        for (int x=0; x<WorldSizeX; ++x) {
            g_TileMap   [(y * WorldSizeX) + x].tile_below    = StdTileOffset::Sandy;
            g_TerrainMap[(y * WorldSizeX) + x].class_below   = TerrainClass::Sandy;
        }
    }

    // carve a some grass...
    for (int y=3; y<3+12; y+=1) {
        for (int x=3; x<3+12; x+=1) {
            g_TileMap        [(y * WorldSizeX) + x].tile_below  = StdTileOffset::Grassy;
        }
    }

}


xString xGetTempDir();

void OpenWorldEnviron::InitScene()
{
    // cut sets out of the source and paste them into a properly-formed TextureAtlas.

    // Complete Sets are 64 px wide and 96 px tall (32px set + 64px set)
    // Within those are several subsets... there's a text file describing them, search for rpgmaker.

    int2 setSize    = {64, 96};
    int2 tileSize   = {32, 32};

    if (1) {
        xBitmapData  pngtex_a1;
        xBitmapData  pngtex_a2;
        png_LoadFromFile(pngtex_a1, FindAsset("sheets/tiles/world_a1_20120604_1883840417.png"));
        png_LoadFromFile(pngtex_a2, FindAsset("sheets/tiles/world_a2_20120604_1478571129.png"));

        TextureAtlas atlas;
        atlas.Init(tileSize);

        //auto sizeInSets = pngtex_a2.size / setSize;
        // grabs everything:
        //for (int cur_set_y=0; cur_set_y < sizeInSets.y; ++cur_set_y) {
        //    for (int cur_set_x=0; cur_set_x < sizeInSets.x; ++cur_set_x) {
        //        int2 setToGrab  = { cur_set_x, cur_set_y };
        //        auto topLeft = setToGrab * setSize;
        //        topLeft.y += 32;          // grab the area set.
        //        topLeft += { 16, 16 };    // skip the transitions.
        //
        //        imgtool::AddTileToAtlas(atlas, pngtex, topLeft);
        //    }
        //}

        int2 offset_solid = {16, 32+16};

        imgtool::AddEmptyTileToAtlas(atlas);

        // Water!  (2nd and third ones are animation states)
        imgtool::AddTileToAtlas(atlas, pngtex_a1, (int2{0,0} * setSize) + offset_solid);
        //imgtool::AddTileToAtlas(atlas, pngtex_a1, (int2{1,0} * setSize) + offset_solid);
        //imgtool::AddTileToAtlas(atlas, pngtex_a1, (int2{2,0} * setSize) + offset_solid);

        // sand followed by grass.
        imgtool::AddTileToAtlas(atlas, pngtex_a2, (int2{0,1} * setSize) + offset_solid);
        imgtool::AddTileToAtlas(atlas, pngtex_a2, (int2{0,0} * setSize) + offset_solid);

        atlas.Solidify();
        x_png_enc pngenc;
        pngenc.WriteImage(atlas);

        auto tempdir = xGetTempDir();
        pngenc.SaveImage(tempdir + "/atlas.png");

        g_GroundLayerAbove.SetSourceTexture(atlas);
        g_GroundLayerBelow.SetSourceTexture(atlas);
    }

    WorldMap_Procgen();

    g_GroundLayerBelow.SetDataOffsetUV(offsetof(TileMapItem, tile_below) / 4);
    g_GroundLayerAbove.SetDataOffsetUV(offsetof(TileMapItem, tile_above) / 4);

    g_GroundLayerBelow.PopulateUVs(g_TileMap, {0,0});
    g_GroundLayerAbove.PopulateUVs(g_TileMap, {0,0});

    fmod_CreateMusic(s_music_world, FindAsset("Audio/Music/ff2over.s3m"));
}

bool s_showLayer_above = 1;
bool s_showLayer_below = 1;

void OpenWorldEnviron::Tick()
{
    ImGui::Checkbox("Show Above-Ground Layer", &s_showLayer_above);
    ImGui::Checkbox("Show Below-Ground Layer", &s_showLayer_below);

    g_GroundLayerBelow.CenterViewOn({ g_ViewCamera.m_Eye.x, g_ViewCamera.m_Eye.y });
    g_GroundLayerAbove.CenterViewOn({ g_ViewCamera.m_Eye.x, g_ViewCamera.m_Eye.y });

    g_GroundLayerBelow.PopulateUVs(g_TileMap);
    g_GroundLayerAbove.PopulateUVs(g_TileMap);

    g_GroundLayerAbove.m_enableDraw = s_showLayer_above;
    g_GroundLayerBelow.m_enableDraw = s_showLayer_below;

    fmod_Play       (s_music_world);
    fmod_SetMute    (s_music_world, g_settings_audio.bgm_muted);
    fmod_SetVolume  (s_music_world, g_settings_audio.bgm_volume);

    ImVec2 buttonSize       = { 18, 13 };

    ImVec2 uv0_unmuted      = { 0.5f, 0.0f };
    ImVec2 uv1_unmuted      = { 1.0f, 1.0f };
    ImVec2 uv0_muted        = { 0.0f, 0.0f };
    ImVec2 uv1_muted        = { 0.5f, 1.0f };

    bool mutepress = ImGui::ImageButton((ImTextureID)s_gui_tex.SoundIcon.gpures.m_driverData_view, buttonSize,
        g_settings_audio.bgm_muted ? uv0_muted : uv0_unmuted,
        g_settings_audio.bgm_muted ? uv1_muted : uv1_unmuted,
        -1      // padding
    );

    if (mutepress) {
        g_settings_audio.bgm_muted = !g_settings_audio.bgm_muted;
        MarkUserSettingsDirty();
    }

    ImGui::SameLine();
    ImGui::SliderFloat("BGM Volume", &g_settings_audio.bgm_volume, 0, 1.0f);
}
