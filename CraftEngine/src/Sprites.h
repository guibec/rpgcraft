#pragma once

#include "x-gpu-ifc.h"
#include "Entity.h"
#include "UniformMeshes.h"

// ------------------------------------------------------------------------------------------------
// struct TileMapVertex
// ------------------------------------------------------------------------------------------------
//  * Used to define the static (generally unchanging) visible tile map.
//  * The UVs are hard-coded to always go from 0.0f to 1.0f across each quad.
//
// Actual UVs are calculated by the pixel shader -- it reads from tex_tile_ids to determine
// the base offset of the tile, and then adds the VS-interpolated value range 0.0f - 1.0f to that
// to smaple the texture.
//
// Note: Requires multi-resource support by the GPU.  No big deal for deaktops and consoles.  May
//       not be widely available on mobile devices; or maybe it will be by the time we're interested
//       in considering shipping the title for mobile.  So just going to assume multi-texture support
//       for now... --jstine

struct TileMapVertex {
	vFloat3		xyz;
	vFloat2		uv;
};

// xyz should probably fixed.  Only the camera and the UVs need to change.
// UV, Lighting should be independenty stored in the future, to allow them to be updated at different update intervals.
//    * 30fps for UV, 10fps for lighting, etc.

struct TileMapVertexLit {
	vFloat3		xyz;
	vFloat2		uv;
	//vFloat4     rgba;		// current light intensity
};

class PlayerSprite
{
private:
	NONCOPYABLE_OBJECT(PlayerSprite);

public:
	GPU_InputDesc			gpu_layout_sprite;

public:
	EntityGid_t			m_gid;
	float2				m_position;
	float				m_frame_timeout;
	int					m_frame_id;
	int					m_anim_dir;
	int					m_char_type;

public:
	static void LoadStaticAssets();

public:
	PlayerSprite();

public:
	void Tick(u32 order, float deltaTime);
	void Draw(float zorder) const;
};

extern GPU_ShaderVS				g_ShaderVS_Spriter;
extern GPU_ShaderFS				g_ShaderFS_Spriter;
extern GPU_TextureResource2D	tex_chars;

