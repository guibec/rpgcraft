#pragma once

#include "x-gpu-ifc.h"
#include "Entity.h"
#include "UniformMeshes.h"

class ViewCamera
{
public:
	EntityGid_t				m_gid;
	u128					m_Eye;
	u128					m_At;
	u128					m_Up;			// X is angle.  Y is just +/- (orientation)? Z is unused?
	GPU_ViewCameraConsts	m_Consts;

	ViewCamera() {
	}

	void Reset();
	virtual void Tick();
};

class TileMapLayer
{
public:
	// todo - GPU references which are shared btween all instances of TileMapLayer.
	GPU_InputDesc			gpu_layout_tilemap;
	GPU_VertexBuffer		mesh_tile;

public:
	EntityGid_t				m_gid;
	GPU_TextureResource2D	tex_floor;
	GPU_VertexBuffer		mesh_worldViewTileID;

	int ViewMeshSizeX			= 24;
	int ViewMeshSizeY			= 24;
	int ViewInstanceCount		= 0;
	int ViewVerticiesCount		= 0;

public:
	TileMapLayer();
	void PopulateUVs();
	void SceneInit(const char* script_objname);

	virtual void Tick();
	virtual void Draw() const;
};


extern ViewCamera		g_ViewCamera;
extern TileMapLayer*	g_TileMap;

extern GPU_ShaderVS			g_ShaderVS_Tiler;
extern GPU_ShaderFS			g_ShaderFS_Tiler;
extern GPU_ConstantBuffer	g_cnstbuf_TileMap;

static const int TileSizeX = 8;
static const int TileSizeY = 8;

// TODO: Make this dynamic ...

//static const int ViewMeshSizeX		= 128;
//static const int ViewMeshSizeY		= 96;

static const int WorldSizeX		= 1024;
static const int WorldSizeY		= 1024;


extern void WorldMap_Procgen();
