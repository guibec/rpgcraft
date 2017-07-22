

Texture2D		txHeightMap : register( t0 );
SamplerState	samLinear	: register( s0 );

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
//	matrix World;
	matrix View;
	matrix Projection;
}

cbuffer ConstantBuffer1 : register( b1 )
{
	float2	TileMapDisp;
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
	//VS_OUTPUT output = (VS_OUTPUT)0;
	//output.Pos = mul( Pos, World );
	//output.Pos = mul( output.Pos, View );
	//output.Pos = mul( output.Pos, Projection );
	//output.Color = Color;
	//return output;

	VS_OUTPUT outp;
	outp.Pos	= float4(input.Pos, 1.0f) * float4(0.7f, 0.86f, 1.0f, 1.0f);
	//outp.Pos	= float4(input.Pos, 1.0f);
	outp.Color	= float4(input.UV, 0.0f, 1.0f);
	outp.UV		= input.UV;
	return outp;

	//float4 Color = input.Color * float4(1.0f, 1.0f, 1.0f, 1.0f);
	//
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_Target
{
	//return float4( input.Pos.xy * 0.001f, 0.0f, 1.0f );   
	return txHeightMap.Sample( samLinear, input.UV );

	//return input.Color; //float4( 1.0f, 1.0f, 0.0f, 1.0f );    // Yellow, with Alpha = 1input.Color;
}
