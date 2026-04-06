BLUR_Y = """
//Cg
//
//Cg profile arbvp1 arbfp1

void vshader(float4 vtx_position : POSITION,
             float2 vtx_texcoord : TEXCOORD0,
             out float4 l_position : POSITION,
             out float2 l_texcoord0 : TEXCOORD0,
             uniform float4 texpad_src,
             uniform float4x4 mat_modelproj)
{
  l_position = mul(mat_modelproj, vtx_position);
  l_texcoord0 = vtx_texcoord * texpad_src.xy * 2;
}


void fshader(float2 l_texcoord0 : TEXCOORD0,
             out float4 o_color : COLOR,
             uniform float2 texpix_src,
             uniform float4 texpad_src,
             uniform sampler2D k_src : TEXUNIT0)
{
  float pad = texpad_src.y * 2;
  float3 offset = float3(1.0*texpix_src.y, 2.0*texpix_src.y, 3.0*texpix_src.y);
  o_color  = tex2D(k_src, l_texcoord0);
  o_color += tex2D(k_src, float2(l_texcoord0.x, l_texcoord0.y - offset.z));
  o_color += tex2D(k_src, float2(l_texcoord0.x, l_texcoord0.y - offset.y));
  o_color += tex2D(k_src, float2(l_texcoord0.x, l_texcoord0.y - offset.x));
  o_color += tex2D(k_src, float2(l_texcoord0.x, min(l_texcoord0.y + offset.x, pad)));
  o_color += tex2D(k_src, float2(l_texcoord0.x, min(l_texcoord0.y + offset.y, pad)));
  o_color += tex2D(k_src, float2(l_texcoord0.x, min(l_texcoord0.y + offset.z, pad)));
  o_color /= 7;
  o_color.w = 1;
}
"""
