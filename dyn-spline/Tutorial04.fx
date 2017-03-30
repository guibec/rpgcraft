//--------------------------------------------------------------------------------------
// File: Tutorial04.fx
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
}

//--------------------------------------------------------------------------------------
struct VS_INPUT
{	
	float3 Pos   : POSITION;
	float4 Color : COLOR0;
};

//--------------------------------------------------------------------------------------
struct VS_OUTPUT
{
	float4 Pos   : SV_POSITION;
	float4 Color : COLOR0;
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

	float4 Color = input.Color * float4(1.0f, 1.0f, 1.0f, 1.0f);

	VS_OUTPUT outp;
	outp.Pos	= float4(input.Pos, 1.0f);
	outp.Color	= input.Color;
	return outp;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( VS_OUTPUT input ) : SV_Target
//float4 PS( float4 Pos : SV_POSITION ) : SV_Target
{
	return input.Color; //float4( 1.0f, 1.0f, 0.0f, 1.0f );    // Yellow, with Alpha = 1input.Color;
}
