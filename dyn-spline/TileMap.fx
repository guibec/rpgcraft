

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

	float4	TileAlignedDisp;
}

//--------------------------------------------------------------------------------------
struct VS_INPUT
{	
	float3 Pos		: POSITION;
	float2 UV		: TEXCOORD0;
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
VS_OUTPUT VS( VS_INPUT input )
{
	VS_OUTPUT outp;
	outp.Pos = float4(input.Pos, 1.0f);
	outp.Pos.xy += TileAlignedDisp;
	outp.Pos = mul( outp.Pos, View );
	outp.Pos = mul( outp.Pos, Projection );

	// just for diag/testing ...
	//outp.Pos	= float4(input.Pos, 1.0f) * float4(0.7f, 0.86f, 1.0f, 1.0f);
	//outp.Pos	= float4(input.Pos, 1.0f);

	outp.Color	= float4(input.UV, 0.0f, 1.0f);
	outp.UV		= input.UV;
	return outp;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_Target
{
	//return float4( input.Pos.xy * 0.001f, 0.0f, 1.0f );   
	return txHeightMap.Sample( samLinear, input.UV );

	//return float4( 1.0f, 1.0f, 0.0f, 1.0f );    // Yellow, with Alpha = 1input.Color;
}
