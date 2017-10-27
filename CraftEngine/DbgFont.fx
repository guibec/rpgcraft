

Texture2D		txHeightMap : register( t0 );
SamplerState	samLinear	: register( s0 );

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix View;
	matrix Projection;
}

cbuffer ConstantBuffer : register( b1 )
{
	// TileAlignedDisp - an optimization parameter meant to allow static mesh vertex coordinates.
	//    (UV and RGBA/Lighting must still be dynamic)

	// Summary: The viewable tile map is typically only a subset of the entire world -- a sliding window.
	//    The tilemap mesh vertex positions are static but the UV/Lighting information is dynamic.
	//    Mesh positions are offset to align with the EYE/AT matrix via this TileAlignedDisp parameter.
	// Non-Optimized operation:
	//    This value can be set to 0 in the frontend and the full mesh data can be
	//    populated on every frame.

	float2		SrcTexTileSizeUV;
	uint2 		SrcTexSizeInTiles;
	uint2		TileMapSizeXY;
	float2		TileSize;
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
	uint   TileID	: TileID;
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
	// Tile vertices are specified in normalized units from 0.0f to 1.0f,
	// which makes math for translating the values into their display position pretty simple.
	// But as a consequence, the Y coordinate must be inverted to conform to standard UP+Y coords.

	VS_OUTPUT outp;

	uint2  tile_xy = uint2( instID % TileMapSizeXY.x, instID / TileMapSizeXY.x);
	float2 incr_xy = TileSize; //(2.0f / TileMapSizeXY) * ProjectionScale;		// 2.0f to map tiles to -1.0 to 1.0 coordinates
	float2 disp_xy = (tile_xy * incr_xy); // + float2(-1.0f, -1.0f);

	// Position Calculation
	// I guess this could be done as a single translation matrix, but matricies are still annoying chunks of
	// black box data to me.  --jstine

	outp.Pos	 = float4(input.Pos, 1.0f);
	//outp.Pos.xy += -0.5f;					// setup for scaling with 0,0 as origin.
	outp.Pos.xy *= incr_xy;					// scale to correct size
	//outp.Pos.xy += (incr_xy/2);				// re-orientate according to upper-left corner of tile
	//outp.Pos.xy += ProjectionXY;			// translate into sheet display position
	outp.Pos.xy += disp_xy;					// translate into tile-on-sheet display position

	//outp.Pos.y	*= -1.0f;					// +Y is UP!
	//outp.Pos	 = mul( outp.Pos, View );

	outp.Pos	 = mul( outp.Pos, Projection );


	// Texture UV Calculation
	float2  incr_set_uv = 1.0f / SrcTexSizeInTiles;
	uint2   tiletex_uv  = uint2( input.TileID % SrcTexSizeInTiles.x, 0);
	outp.UV		 = input.UV * SrcTexTileSizeUV;
	outp.UV	    += float2(tiletex_uv) * incr_set_uv;

	// Color & Lighting Calculation  (not implemented)
	outp.Color	= float4(1.0f, 0.0f, 0.0f, 1.0f);

	return outp;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_Target
{
	float4 frag = txHeightMap.Sample( samLinear, input.UV );

	// without [branch], clipping judgement fails when doing minification sampling (might be specific to Intel GPU)
	[branch]
	if (frag.a == 0.0f) {
		clip(-1);
	}
	return frag;
}
