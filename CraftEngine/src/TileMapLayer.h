#pragma once

#include "x-gpu-ifc.h"
#include "Entity.h"
#include "UniformMeshes.h"

class ViewCamera
{
public:
	EntityGid_t				m_gid;
	float4					m_Eye;
	float4					m_At;
	float4					m_Up;			// X is angle.  Y is just +/- (orientation)? Z is unused?
	float2					m_frustrum_in_tiles;
	float					m_aspect;
	GPU_ViewCameraConsts	m_Consts;

	ViewCamera() {
	}

	void			Reset		();
	void			SetEyeAt	(const float2& xy);
	float4			ClientToWorld(const int2& clientPosInPix);

	virtual void Tick();

};

class TileMapLayer
{
public:
	struct GPU_TileMapConstants
	{
		vFloat2 TileAlignedDisp;		// TODO: move calculation of this to shader.
		vInt2	SrcTexSizeInTiles;
		vFloat2	SrcTexTileSizeUV;
		u32		TileMapSizeX;
		u32		TileMapSizeY;
	};

public:
	EntityGid_t				m_gid;

	// No effort is made here to break out shared resources, such as tile or layout.
	// A meaningful advantage would only become worthwhile if 20 or more tilemap layers
	// are being used during a scene.  Seems highly unlikely --jstine

	struct {
		GPU_InputDesc			layout_tilemap;
		GPU_TextureResource2D	tex_floor;
		GPU_VertexBuffer		mesh_tile;
		GPU_DynVsBuffer			mesh_worldViewTileID;
		GPU_TileMapConstants	consts;
	} gpu;

	int ViewMeshSizeX			= 24;
	int ViewMeshSizeY			= 24;
	int ViewInstanceCount		= 0;
	int ViewVerticiesCount		= 0;

public:
	TileMapLayer();
	void PopulateUVs(const int2& viewport_offset);
	void SceneInit(const char* script_objname);

	virtual void Tick();
	virtual void Draw() const;
};


extern ViewCamera		g_ViewCamera;
extern TileMapLayer		g_TileMap;

extern GPU_ShaderVS			g_ShaderVS_Tiler;
extern GPU_ShaderFS			g_ShaderFS_Tiler;
extern GPU_ConstantBuffer	g_cnstbuf_TileMap;

static const int TileSizeX = 8;
static const int TileSizeY = 8;

// TODO: Make this dynamic ...

static const int WorldSizeX		= 1024;
static const int WorldSizeY		= 1024;


extern void WorldMap_Procgen();
