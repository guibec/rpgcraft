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
struct VS_OUTPUT
{
    float4 Pos   : SV_POSITION;
    float4 Color : COLOR0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
float4 VS( float4 Pos : POSITION ) : SV_POSITION // float4 Color : COLOR )
{
    //VS_OUTPUT output = (VS_OUTPUT)0;
    //output.Pos = mul( Pos, World );
    //output.Pos = mul( output.Pos, View );
    //output.Pos = mul( output.Pos, Projection );
    //output.Color = Color;
    //return output;

	return Pos;

	//return Pos * float4(1.0f, -1.0f, 1.0f, 1.0f);
	//float4 pos_inv_y = Pos * float4(1.0f, -1.0f, 1.0f, 1.0f);
	//return pos_inv_y;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
//float4 PS( VS_OUTPUT input ) : SV_Target
float4 PS( float4 Pos : SV_POSITION ) : SV_Target
{
    return float4( 1.0f, 1.0f, 0.0f, 1.0f );    // Yellow, with Alpha = 1input.Color;
}
