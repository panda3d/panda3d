BLOOM_I = """
//Cg
//
// blend.rgb
//
//   This shader converts to black-and-white before calculating
//   scene brightness.  To do this, it uses a weighted average of
//   R,G,B.  The blend parameter controls the weighting.
//
// desat.x
//
//   Desaturation level.  If zero, the bloom's color is equal to
//   the color of the input pixel.  If one, the bloom's color is
//   white.
//
// trigger.x
//
//   Must be equal to mintrigger.
//
//   mintrigger is the minimum brightness to trigger a bloom,
//   and maxtrigger is the brightness at which the bloom
//   reaches maximum intensity.
//
// trigger.y
//
//   Must be equal to (1.0/(maxtrigger-mintrigger)) where
//
//   mintrigger is the minimum brightness to trigger a bloom,
//   and maxtrigger is the brightness at which the bloom
//   reaches maximum intensity.
//


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
  l_position=mul(mat_modelproj, vtx_position);
  float2 c = vtx_texcoord * texpad_src.xy * 2;
  float4 offs = texpix_src * 0.5;
  l_texcoordNW = c + float2( offs.x, -offs.y);
  l_texcoordNE = c + float2( offs.x,  offs.y);
  l_texcoordSW = c + float2(-offs.x, -offs.y);
  l_texcoordSE = c + float2(-offs.x,  offs.y);
}

void fshader(float2 l_texcoordNW : TEXCOORD0,
             float2 l_texcoordNE : TEXCOORD1,
             float2 l_texcoordSW : TEXCOORD2,
             float2 l_texcoordSE : TEXCOORD3,
             uniform sampler2D k_src : TEXUNIT0,
             out float4 o_color : COLOR,
             uniform float4 k_blend,
             uniform float4 k_trigger,
             uniform float4 k_desat
             )
{
  float4 inputNW = tex2D(k_src, l_texcoordNW) - float4(0,0,0,0.5);
  float briteNW = dot(inputNW, k_blend);
  float scaleNW = saturate((briteNW - k_trigger.x) * k_trigger.y);
  float4 colorNW = scaleNW * lerp(inputNW, float4(1,1,1,1), k_desat.x);

  float4 inputNE = tex2D(k_src, l_texcoordNE) - float4(0,0,0,0.5);
  float briteNE = dot(inputNE, k_blend);
  float scaleNE = saturate((briteNE - k_trigger.x) * k_trigger.y);
  float4 colorNE = scaleNE * lerp(inputNE, float4(1,1,1,1), k_desat.x);

  float4 inputSW = tex2D(k_src, l_texcoordSW) - float4(0,0,0,0.5);
  float briteSW = dot(inputSW, k_blend);
  float scaleSW = saturate((briteSW - k_trigger.x) * k_trigger.y);
  float4 colorSW = scaleSW * lerp(inputSW, float4(1,1,1,1), k_desat.x);

  float4 inputSE = tex2D(k_src, l_texcoordSE) - float4(0,0,0,0.5);
  float briteSE = dot(inputSE, k_blend);
  float scaleSE = saturate((briteSE - k_trigger.x) * k_trigger.y);
  float4 colorSE = scaleSE * lerp(inputSE, float4(1,1,1,1), k_desat.x);

  o_color = (colorNW + colorNE + colorSW + colorSE) * 0.25;
}

"""
