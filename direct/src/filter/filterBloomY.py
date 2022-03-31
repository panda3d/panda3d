BLOOM_Y = """
//Cg

void vshader(float4 vtx_position : POSITION,
             out float4 l_position : POSITION,
             out float4 l_texcoord0 : TEXCOORD0,
             out float4 l_texcoord1 : TEXCOORD1,
             out float4 l_texcoord2 : TEXCOORD2,
             uniform float4 texpad_src,
             uniform float4 texpix_src,
             uniform float4x4 mat_modelproj)
{
  l_position=mul(mat_modelproj, vtx_position);
  float2 c=(vtx_position.xz * texpad_src.xy) + texpad_src.xy;
  float offset = texpix_src.y;
  float pad = texpad_src.y * 2;
  l_texcoord0 = float4(min(c.y-offset* -4, pad), min(c.y-offset* -3, pad), min(c.y-offset* -2, pad), c.x);
  l_texcoord1 = float4(min(c.y-offset* -1, pad), c.y-offset*  0, c.y-offset*  1, c.x);
  l_texcoord2 = float4(c.y-offset*  2, c.y-offset*  3, c.y-offset*  4, c.x);
}

void fshader(float4 l_texcoord0 : TEXCOORD0,
             float4 l_texcoord1 : TEXCOORD1,
             float4 l_texcoord2 : TEXCOORD2,
             uniform sampler2D k_src : TEXUNIT0,
             uniform float4 k_intensity,
             out float4 o_color : COLOR) {
  float4 color = float4(0,0,0,0);
  color  =  50 * tex2D(k_src, l_texcoord0.wx);
  color += 100 * tex2D(k_src, l_texcoord0.wy);
  color += 150 * tex2D(k_src, l_texcoord0.wz);
  color += 200 * tex2D(k_src, l_texcoord1.wx);
  color += 200 * tex2D(k_src, l_texcoord1.wy);
  color += 200 * tex2D(k_src, l_texcoord1.wz);
  color += 150 * tex2D(k_src, l_texcoord2.wx);
  color += 100 * tex2D(k_src, l_texcoord2.wy);
  color +=  50 * tex2D(k_src, l_texcoord2.wz);
  o_color = color / 1200.0;
  o_color = o_color * k_intensity;
}
"""
