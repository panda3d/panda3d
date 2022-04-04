DOWN_4 = """
//Cg

void vshader(float4 vtx_position : POSITION,
             float2 vtx_texcoord : TEXCOORD0,
             out float4 l_position : POSITION,
             out float2 l_texcoordNW : TEXCOORD0,
             out float2 l_texcoordNE : TEXCOORD1,
             out float2 l_texcoordSW : TEXCOORD2,
             out float2 l_texcoordSE : TEXCOORD3,
             uniform float4 texpad_src,
             uniform float4 texpix_src,
             uniform float4x4 mat_modelproj)
{
  l_position = mul(mat_modelproj, vtx_position);
  float2 c = vtx_texcoord * texpad_src.xy * 2;
  l_texcoordNW = c + float2( texpix_src.x, -texpix_src.y);
  l_texcoordNE = c + float2( texpix_src.x,  texpix_src.y);
  l_texcoordSW = c + float2(-texpix_src.x, -texpix_src.y);
  l_texcoordSE = c + float2(-texpix_src.x,  texpix_src.y);
}

void fshader(float2 l_texcoordNW : TEXCOORD0,
             float2 l_texcoordNE : TEXCOORD1,
             float2 l_texcoordSW : TEXCOORD2,
             float2 l_texcoordSE : TEXCOORD3,
             uniform sampler2D k_src : TEXUNIT0,
             out float4 o_color : COLOR) {
  float4 colorNW = tex2D(k_src, l_texcoordNW);
  float4 colorNE = tex2D(k_src, l_texcoordNE);
  float4 colorSW = tex2D(k_src, l_texcoordSW);
  float4 colorSE = tex2D(k_src, l_texcoordSE);
  o_color = (colorNW + colorNE + colorSW + colorSE);
}
"""
