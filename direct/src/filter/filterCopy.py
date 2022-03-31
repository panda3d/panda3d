COPY = """
//Cg


void vshader(float4 vtx_position : POSITION,
             out float4 l_position : POSITION,
             out float2 l_texcoord : TEXCOORD0,
             uniform float4 texpad_src,
             uniform float4x4 mat_modelproj)
{
  l_position=mul(mat_modelproj, vtx_position);
  l_texcoord = (vtx_position.xz * texpad_src.xy) + texpad_src.xy;
}

void fshader(float2 l_texcoord : TEXCOORD0,
             uniform sampler2D k_src : TEXUNIT0,
             out float4 o_color : COLOR)
{
  o_color = tex2D(k_src, l_texcoord);
}
"""
