

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

cbuffer ConstantBuffer1 : register( b1 )
{
	// TileAlignedDisp - an optimization parameter meant to allow static mesh vertex coordinates.
	//    (UV and RGBA/Lighting must still be dynamic)

	// Summary: The viewable tile map is typically only a subset of the entire world -- a sliding window.
	//    The tilemap mesh vertex positions are static but the UV/Lighting information is dynamic.
	//    Mesh positions are offset to align with the EYE/AT matrix via this TileAlignedDisp parameter.
	// Non-Optimized operation:
	//    This value can be set to 0 in the frontend and the full mesh data can be
	//    populated on every frame.

	float2	TileAlignedDisp;
	uint2 	SrcTexSizeInTiles;
	float2	SrcTexTileSizeUV;
	uint2   TileMapSizeXY;
}

//--------------------------------------------------------------------------------------
struct VS_INPUT_TILEMAP
{
	float3 Pos		: POSITION;
	float2 UV		: TEXCOORD0;
	uint   TileID	: TileID;
	float2 Color	: COLOR;
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
// Calculates the tile position using the following inputs:
//  * a single normalized tile mesh (0.0f->1.0f)
//  * uniform texture atlus, described by CB1
//  * per-instance data for TileID and lighting.
//  * Instance ID from which tile onscreen position can be calculated.
//

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
	float2 incr_xy = float2(1.0f, 1.0f);
	float2 disp_xy = (TileMapSizeXY * -0.5f) + (tile_xy * incr_xy) - 0.5f;

	// Position Calculation
	outp.Pos	 = float4(input.Pos.xy, 1.0f, 1.0f);
	outp.Pos.xy	+= disp_xy;
	outp.Pos.xy += TileAlignedDisp;
	outp.Pos.y	*= -1.0f;		// +Y is UP!
	outp.Pos	 = mul( outp.Pos, View );
	outp.Pos	 = mul( outp.Pos, Projection );

	// Texture UV Calculation
	float2  incr_set_uv = 1.0f / SrcTexSizeInTiles;
	uint2   tiletex_uv  = uint2( input.TileID % SrcTexSizeInTiles.x, input.TileID / SrcTexSizeInTiles.x);
	outp.UV		= input.UV;
	outp.UV		= input.UV * SrcTexTileSizeUV;
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
	return txHeightMap.Sample( samLinear, input.UV );

	//return float4( 1.0f, 1.0f, 0.0f, 1.0f );    // Yellow, with Alpha = 1input.Color;
}
