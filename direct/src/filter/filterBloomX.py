BLOOM_X = """
//Cg

void vshader(float4 vtx_position : POSITION,
             float2 vtx_texcoord : TEXCOORD0,
             out float4 l_position : POSITION,
             out float4 l_texcoord0 : TEXCOORD0,
             out float4 l_texcoord1 : TEXCOORD1,
             out float4 l_texcoord2 : TEXCOORD2,
             uniform float4 texpad_src,
             uniform float4 texpix_src,
             uniform float4x4 mat_modelproj)
{
  l_position = mul(mat_modelproj, vtx_position);
  float2 c = vtx_texcoord * texpad_src.xy * 2;
  float offset = texpix_src.x;
  float pad = texpad_src.x * 2;
  l_texcoord0 = float4(min(c.x-offset* -4, pad), min(c.x-offset* -3, pad), min(c.x-offset* -2, pad), c.y);
  l_texcoord1 = float4(min(c.x-offset* -1, pad), c.x-offset*  0, c.x-offset*  1, c.y);
  l_texcoord2 = float4(c.x-offset*  2, c.x-offset*  3, c.x-offset*  4, c.y);
}

void fshader(float4 l_texcoord0 : TEXCOORD0,
             float4 l_texcoord1 : TEXCOORD1,
             float4 l_texcoord2 : TEXCOORD2,
             uniform sampler2D k_src : TEXUNIT0,
             out float4 o_color : COLOR) {
  float4 color = float4(0,0,0,0);
//  color  =  10 * tex2D(k_src, l_texcoord0.xw);
//  color +=  45 * tex2D(k_src, l_texcoord0.yw);
//  color += 120 * tex2D(k_src, l_texcoord0.zw);
//  color += 210 * tex2D(k_src, l_texcoord1.xw);
//  color += 252 * tex2D(k_src, l_texcoord1.yw);
//  color += 210 * tex2D(k_src, l_texcoord1.zw);
//  color += 120 * tex2D(k_src, l_texcoord2.xw);
//  color +=  45 * tex2D(k_src, l_texcoord2.yw);
//  color +=  10 * tex2D(k_src, l_texcoord2.zw);
//  o_color = color / 1022.0;

  color  =  50 * tex2D(k_src, l_texcoord0.xw);
  color += 100 * tex2D(k_src, l_texcoord0.yw);
  color += 150 * tex2D(k_src, l_texcoord0.zw);
  color += 200 * tex2D(k_src, l_texcoord1.xw);
  color += 200 * tex2D(k_src, l_texcoord1.yw);
  color += 200 * tex2D(k_src, l_texcoord1.zw);
  color += 150 * tex2D(k_src, l_texcoord2.xw);
  color += 100 * tex2D(k_src, l_texcoord2.yw);
  color +=  50 * tex2D(k_src, l_texcoord2.zw);
  o_color = color / 1200.0;
}
"""
