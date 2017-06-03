

struct VS_INPUT_COLOR
{	
	float3 Pos		: POSITION;
	float2 Color	: COLOR;
};

struct VS_INPUT_TEX1
{	
	float3 Pos		: POSITION;
	float2 UV		: TEXCOORD0;
};

struct VS_INPUT_COLORTEX1
{	
	float3 Pos		: POSITION;
	float2 Color	: COLOR;
	float2 UV		: TEXCOORD0;
};

struct VS_INPUT_COLORTEX4
{	
	float3 Pos		: POSITION;
	float2 Color	: COLOR;
	float2 UV		: TEXCOORD3;
};

struct VS_OUTPUT
{
	float4 Pos		: SV_POSITION;
	float4 Color 	: COLOR0;
};

VS_OUTPUT VertexBufferLayout_Color( VS_INPUT_COLOR input )
{
	VS_OUTPUT result;
	result.Pos = float4(input.Pos, 0.0f);
	result.Color = float4(0.0f, 0.0f, 0.0f, 0.0f);
	return result;
}

VS_OUTPUT VertexBufferLayout_Tex1( VS_INPUT_TEX1 input )
{
	VS_OUTPUT result;
	result.Pos = float4(input.Pos, 0.0f);
	result.Color = float4(0.0f, 0.0f, 0.0f, 0.0f);
	return result;
}

VS_OUTPUT VertexBufferLayout_ColorTex1( VS_INPUT_COLORTEX1 input )
{
	VS_OUTPUT result;
	result.Pos = float4(input.Pos, 0.0f);
	result.Color = float4(0.0f, 0.0f, 0.0f, 0.0f);
	return result;
}

VS_OUTPUT VertexBufferLayout_ColorTex4( VS_INPUT_COLORTEX4 input )
{
	VS_OUTPUT result;
	result.Pos = float4(input.Pos, 0.0f);
	result.Color = float4(0.0f, 0.0f, 0.0f, 0.0f);
	return result;
}

