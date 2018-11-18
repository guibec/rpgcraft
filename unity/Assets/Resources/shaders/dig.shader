// Upgrade NOTE: replaced 'mul(UNITY_MATRIX_MVP,*)' with 'UnityObjectToClipPos(*)'

Shader "dig" 
{ 
	Properties
	{
	  _MainTex ("Texture Image", 2D) = "white" {} 
	  _Destruction ("how much destruction", Float) = 0.5
	}


   SubShader { // Unity chooses the subshader that fits the GPU best
      Pass { // some shaders require multiple passes
         CGPROGRAM // here begins the part in Unity's Cg
 
         #pragma vertex vert 
         #pragma fragment frag

		 uniform sampler2D _MainTex;

		 struct a2v
		 {
			float4 vertex : POSITION;
			float3 normal : NORMAL;
			float4 texcoord : TEXCOORD0;
			float4 color : COLOR;
		 };

		struct v2f 
		{
			float4 position : POSITION;
			float3 normal : NORMAL;
			float4 texcoord : TEXCOORD0;
			float4 color : COLOR;
			
		};

         v2f vert(a2v In)
         {
            v2f Out;
			Out.position = UnityObjectToClipPos(In.vertex);
			Out.texcoord = In.texcoord;
			Out.normal = In.normal;
			Out.color = In.color;
			return Out;
         }
 
		 float4 frag(v2f In) : COLOR
         {
			return tex2D(_MainTex, In.texcoord);
         }
 
         ENDCG // here ends the part in Cg 
      }
   }
}