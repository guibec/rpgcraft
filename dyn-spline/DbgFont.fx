

Texture2D		txHeightMap : register( t0 );
SamplerState	samLinear	: register( s0 );

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	// TileAlignedDisp - an optimization parameter meant to allow static mesh vertex coordinates.
	//    (UV and RGBA/Lighting must still be dynamic)

	// Summary: The viewable tile map is typically only a subset of the entire world -- a sliding window.
	//    The tilemap mesh vertex positions are static but the UV/Lighting information is dynamic.
	//    Mesh positions are offset to align with the EYE/AT matrix via this TileAlignedDisp parameter.
	// Non-Optimized operation:
	//    This value can be set to 0 in the frontend and the full mesh data can be
	//    populated on every frame.

	float2	SrcTexTileSizeUV;
	uint2 	SrcTexSizeInTiles;
	uint2   TileMapSizeXY;
}

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 Pos		: POSITION;
	float2 UV		: TEXCOORD0;
};

struct VS_INPUT_TILEMAP
{
	float3 Pos		: POSITION;
	float2 UV		: TEXCOORD0;
	uint   TileID	: mTileID;
	float4 Color	: COLOR;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
	float4 Pos		: SV_POSITION;
	float4 Color	: COLOR;
	float2 UV		: TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS( VS_INPUT_TILEMAP input, uint instID : SV_InstanceID )		// uint vertexID : SV_VertexID
{
	// note: move floor(View) calculation to shader and remove global.
	//
	//    The XY of View is discarded since the tilemap is already limited to the user's immediate
	//    viewable area.  TileAlignedDisp is used to shift the static mesh to match the view.

	// Tile vertices are specified in normalized units from 0.0f to 1.0f,
	// which makes math for translating the values into their display position pretty simple.
	// But as a consequence, the Y coordinate must be inverted to conform to standard UP+Y coords.

	VS_OUTPUT outp;

	uint2  tile_xy = uint2( instID % TileMapSizeXY.x, instID / TileMapSizeXY.x);
	float2 incr_xy = 2.0f / TileMapSizeXY;
	//float2 disp_xy = (TileMapSizeXY * -0.5f) + (tile_xy * incr_xy) - 0.5f;
	float2 disp_xy = (tile_xy * incr_xy) + float2(-1.0f, -1.0f);

	// Position Calculation
	outp.Pos	 = float4(input.Pos, 1.0f);
	//outp.Pos.xy	+= disp_xy;
	//outp.Pos.y	*= -1.0f;		// +Y is UP!

	outp.Pos.xy *=  2.0f;
	outp.Pos.y	*= -1.0f;		// +Y is UP!
	outp.Pos.xy += float2(-1.0f, 1.0f);
	outp.Pos.xy *= incr_xy;
	outp.Pos.xy  += disp_xy.xy;

	//outp.Pos.xy /= TileMapSizeXY / 2;
	//outp.Pos.xy += TileAlignedDisp;
	//outp.Pos	 = mul( outp.Pos, View );
	//outp.Pos	 = mul( outp.Pos, Projection );

	// Texture UV Calculation
	float2  incr_set_uv = 1.0f / SrcTexSizeInTiles;
	uint2   tiletex_uv  = uint2( input.TileID % SrcTexSizeInTiles.x, 0);
	//outp.UV		= input.UV / float2(128.0f, 1.0f);
	//outp.UV.x += 0.4f;

	outp.UV		= input.UV * SrcTexTileSizeUV;
	//outp.UV.x += 0.4f;
	//outp.UV	   += float2(0.5, 0);
	outp.UV	   += float2(tiletex_uv * incr_set_uv);
	// Color & Lighting Calculation  (not implemented)
	outp.Color	= float4(1.0f, 0.0f, 0.0f, 1.0f);

	return outp;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_Target
{
	//return input.Color;
	//float4 result = txHeightMap.Sample( samLinear, input.UV );
	//result *= input.Color;
	//return result;

	//return float4( input.Pos.xy * 0.001f, 0.0f, 1.0f );
	float4 frag = txHeightMap.Sample( samLinear, input.UV );
	clip(frag != float4(0,0,0,1.0f) ? -1 : 0);
	return frag;
	//return float4( 1.0f, 1.0f, 0.0f, 1.0f );    // Yellow, with Alpha = 1input.Color;
}
