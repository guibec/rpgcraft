

Texture2D		txHeightMap : register( t0 );
SamplerState	samLinear	: register( s0 );

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer0 : register( b0 )
{
	matrix View;
	matrix Projection;
}

cbuffer ConstantBuffer1 : register( b1 )
{
	//matrix World;
	float2   worldpos;
	uint2    TileMapSize;		// TODO - move this to cb0
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

static matrix Identity =
{
	{ 1, 0, 0, 0 },
	{ 0, 1, 0, 0 },
	{ 0, 0, 1, 0 },
	{ 0, 0, 0, 1 }
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
VS_OUTPUT VS( VS_INPUT input )
{
	VS_OUTPUT outp;

	float2 incr_xy = float2(1.0f, 1.0f);
	float2 disp_xy = (TileMapSize * -0.5f) + (worldpos * incr_xy) - 0.5f;

	outp.Pos	 = float4(input.Pos, 1.0f);
	outp.Pos.xy += disp_xy;
	outp.Pos.y	*= -1.0f;		// +Y is UP!
	outp.Pos	 = mul( outp.Pos, View );
	outp.Pos	 = mul( outp.Pos, Projection );

	outp.Color	= float4(input.UV, 0.0f, 1.0f);
	outp.UV		= input.UV;

	float2 texSize;
	float  iggy;
	txHeightMap.GetDimensions(0, texSize.x, texSize.y, iggy);

	// UV input is pixels -- scale according to renderTarget size
	outp.UV      = float2(input.UV) / (float2)texSize;

	return outp;

	//float4 Color = input.Color * float4(1.0f, 1.0f, 1.0f, 1.0f);
	//
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_Target
{
	return txHeightMap.Sample( samLinear, input.UV );
}
