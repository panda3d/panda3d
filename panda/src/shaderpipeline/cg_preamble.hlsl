/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cg_preamble.hlsl
 * @author rdb
 * @date 2020-06-10
 */

/**
 * This file gets loaded as preamble to any Cg shader to be able to compile it
 * through the HLSL front-end of the glslang compiler.  It defines functions
 * and types that were supported in Cg but are not available in HLSL.
 */

// DX10 just maps half to float, but Cg defined it as float>=16.
// This makes glslang recognize them as relaxed-precision floats.
#define half min16float
#define half1 min16float1
#define half2 min16float2
#define half3 min16float3
#define half4 min16float4
#define half1x1 min16float1x1
#define half1x2 min16float1x2
#define half1x3 min16float1x3
#define half1x4 min16float1x4
#define half2x1 min16float2x1
#define half2x2 min16float2x2
#define half2x3 min16float2x3
#define half2x4 min16float2x4
#define half3x1 min16float3x1
#define half3x2 min16float3x2
#define half3x3 min16float3x3
#define half3x4 min16float3x4
#define half4x1 min16float4x1
#define half4x2 min16float4x2
#define half4x3 min16float4x3
#define half4x4 min16float4x4

// Cg also had a fixed-precision type, corresponding to lowp/10-bit.
#define fixed min10float
#define fixed1 min10float1
#define fixed2 min10float2
#define fixed3 min10float3
#define fixed4 min10float4
#define fixed1x1 min10float1x1
#define fixed1x2 min10float1x2
#define fixed1x3 min10float1x3
#define fixed1x4 min10float1x4
#define fixed2x1 min10float2x1
#define fixed2x2 min10float2x2
#define fixed2x3 min10float2x3
#define fixed2x4 min10float2x4
#define fixed3x1 min10float3x1
#define fixed3x2 min10float3x2
#define fixed3x3 min10float3x3
#define fixed3x4 min10float3x4
#define fixed4x1 min10float4x1
#define fixed4x2 min10float4x2
#define fixed4x3 min10float4x3
#define fixed4x4 min10float4x4

// Define these DX10 samplers, used below.  They are removed by a glslang pass.
SamplerState __builtin_sampler;
SamplerComparisonState __builtin_shadow_sampler;

// These sampler types have no syntax in DX9, so we map them to DX10 types.
typedef Texture1D sampler1DShadow;
typedef Texture1DArray<float4> sampler1DARRAY;
typedef Texture1DArray<int4> isampler1DARRAY;
typedef Texture1DArray<uint4> usampler1DARRAY;
typedef Texture2D sampler2DShadow;
typedef Texture2DArray<float4> sampler2DARRAY;
typedef Texture2DArray<int4> isampler2DARRAY;
typedef Texture2DArray<uint4> usampler2DARRAY;
typedef TextureCube samplerCubeShadow;
typedef TextureCubeArray<float4> samplerCUBEARRAY;
typedef TextureCubeArray<int4> isamplerCUBEARRAY;
typedef TextureCubeArray<uint4> usamplerCUBEARRAY;
typedef Buffer<float4> samplerBUF;
typedef Buffer<int4> isamplerBUF;
typedef Buffer<uint4> usamplerBUF;

// Define texture functions not available in HLSL.
float4 shadow1D(sampler1DShadow samp, float3 s) {
  return samp.SampleCmp(__builtin_shadow_sampler, s.x, s.z);
}

float4 shadow1DProj(sampler1DShadow samp, float4 s) {
  return samp.SampleCmp(__builtin_shadow_sampler, s.x / s.w, s.z / s.w);
}

float4 shadow2D(sampler2DShadow samp, float3 s) {
  return samp.SampleCmp(__builtin_shadow_sampler, s.xy, s.z);
}

float4 shadow2DProj(sampler2DShadow samp, float4 s) {
  return samp.SampleCmp(__builtin_shadow_sampler, s.xy / s.w, s.z / s.w);
}

float4 shadowCube(samplerCubeShadow samp, float4 s) {
  return samp.SampleCmp(__builtin_shadow_sampler, s.xyz, s.w);
}

float4 tex1DARRAY(sampler1DARRAY samp, float2 s) {
  return samp.Sample(__builtin_sampler, s);
}

float4 tex1DARRAY(sampler1DARRAY samp, float2 s, int texelOff) {
  return samp.Sample(__builtin_sampler, s, texelOff);
}

float4 tex1DARRAY(sampler1DARRAY samp, float2 s, float dx, float dy) {
  return samp.SampleGrad(__builtin_sampler, s, dx, dy);
}

float4 tex1DARRAY(sampler1DARRAY samp, float2 s, float dx, float dy, int texelOff) {
  return samp.SampleGrad(__builtin_sampler, s, dx, dy, texelOff);
}

float4 tex1DARRAY(sampler1DARRAY samp, float3 s) {
  return samp.SampleCmp(__builtin_shadow_sampler, s.xy, s.z);
}

float4 tex1DARRAY(sampler1DARRAY samp, float3 s, int texelOff) {
  return samp.SampleCmp(__builtin_shadow_sampler, s.xy, s.z, texelOff);
}

int4 tex1DARRAY(isampler1DARRAY samp, float2 s) {
  return samp.Sample(__builtin_sampler, s);
}

int4 tex1DARRAY(isampler1DARRAY samp, float2 s, int texelOff) {
  return samp.Sample(__builtin_sampler, s, texelOff);
}

int4 tex1DARRAY(isampler1DARRAY samp, float2 s, float dx, float dy) {
  return samp.SampleGrad(__builtin_sampler, s, dx, dy);
}

int4 tex1DARRAY(isampler1DARRAY samp, float2 s, float dx, float dy, int texelOff) {
  return samp.SampleGrad(__builtin_sampler, s, dx, dy, texelOff);
}

uint4 tex1DARRAY(usampler1DARRAY samp, float2 s) {
  return samp.Sample(__builtin_sampler, s);
}

uint4 tex1DARRAY(usampler1DARRAY samp, float2 s, int texelOff) {
  return samp.Sample(__builtin_sampler, s, texelOff);
}

uint4 tex1DARRAY(usampler1DARRAY samp, float2 s, float dx, float dy) {
  return samp.SampleGrad(__builtin_sampler, s, dx, dy);
}

uint4 tex1DARRAY(usampler1DARRAY samp, float2 s, float dx, float dy, int texelOff) {
  return samp.SampleGrad(__builtin_sampler, s, dx, dy, texelOff);
}

float4 tex1DARRAYproj(sampler1DARRAY samp, float3 s) {
  return samp.Sample(__builtin_sampler, s.xy / s.z);
}

float4 tex1DARRAYproj(sampler1DARRAY samp, float3 s, int texelOff) {
  return samp.Sample(__builtin_sampler, s.xy / s.z, texelOff);
}

int4 tex1DARRAYproj(isampler1DARRAY samp, float3 s) {
  return samp.Sample(__builtin_sampler, s.xy / s.z);
}

int4 tex1DARRAYproj(isampler1DARRAY samp, float3 s, int texelOff) {
  return samp.Sample(__builtin_sampler, s.xy / s.z, texelOff);
}

uint4 tex1DARRAYproj(usampler1DARRAY samp, float3 s) {
  return samp.Sample(__builtin_sampler, s.xy / s.z);
}

uint4 tex1DARRAYproj(usampler1DARRAY samp, float3 s, int texelOff) {
  return samp.Sample(__builtin_sampler, s.xy / s.z, texelOff);
}

float4 tex1DARRAYproj(sampler1DARRAY samp, float4 s) {
  return samp.SampleCmp(__builtin_shadow_sampler, s.xy / s.w, s.z / s.w);
}

float4 tex1DARRAYproj(sampler1DARRAY samp, float4 s, int texelOff) {
  return samp.SampleCmp(__builtin_shadow_sampler, s.xy / s.w, s.z / s.w, texelOff);
}

int3 tex1DARRAYsize(sampler1DARRAY samp, int lod) {
  uint width, elems, levels;
  samp.GetDimensions(lod, width, elems, levels);
  return int3(width, 0, 0); // sic
}

int3 tex1DARRAYsize(isampler1DARRAY samp, int lod) {
  uint width, elems, levels;
  samp.GetDimensions(lod, width, elems, levels);
  return int3(width, 0, 0); // sic
}

int3 tex1DARRAYsize(usampler1DARRAY samp, int lod) {
  uint width, elems, levels;
  samp.GetDimensions(lod, width, elems, levels);
  return int3(width, 0, 0); // sic
}

float4 tex2DARRAY(sampler2DARRAY samp, float3 s) {
  return samp.Sample(__builtin_sampler, s);
}

float4 tex2DARRAY(sampler2DARRAY samp, float3 s, int2 texelOff) {
  return samp.Sample(__builtin_sampler, s, texelOff);
}

float4 tex2DARRAY(sampler2DARRAY samp, float3 s, float2 dx, float2 dy) {
  return samp.SampleGrad(__builtin_sampler, s, dx, dy);
}

float4 tex2DARRAY(sampler2DARRAY samp, float3 s, float2 dx, float2 dy, int2 texelOff) {
  return samp.SampleGrad(__builtin_sampler, s, dx, dy, texelOff);
}

float4 tex2DARRAY(sampler2DARRAY samp, float4 s) {
  return samp.SampleCmp(__builtin_shadow_sampler, s.xyz, s.w);
}

float4 tex2DARRAY(sampler2DARRAY samp, float4 s, int2 texelOff) {
  return samp.SampleCmp(__builtin_shadow_sampler, s.xyz, s.w, texelOff);
}

int4 tex2DARRAY(isampler2DARRAY samp, float3 s) {
  return samp.Sample(__builtin_sampler, s);
}

int4 tex2DARRAY(isampler2DARRAY samp, float3 s, int2 texelOff) {
  return samp.Sample(__builtin_sampler, s, texelOff);
}

int4 tex2DARRAY(isampler2DARRAY samp, float3 s, float2 dx, float2 dy) {
  return samp.SampleGrad(__builtin_sampler, s, dx, dy);
}

int4 tex2DARRAY(isampler2DARRAY samp, float3 s, float2 dx, float2 dy, int2 texelOff) {
  return samp.SampleGrad(__builtin_sampler, s, dx, dy, texelOff);
}

uint4 tex2DARRAY(usampler2DARRAY samp, float3 s) {
  return samp.Sample(__builtin_sampler, s);
}

uint4 tex2DARRAY(usampler2DARRAY samp, float3 s, int2 texelOff) {
  return samp.Sample(__builtin_sampler, s, texelOff);
}

uint4 tex2DARRAY(usampler2DARRAY samp, float3 s, float2 dx, float2 dy) {
  return samp.SampleGrad(__builtin_sampler, s, dx, dy);
}

uint4 tex2DARRAY(usampler2DARRAY samp, float3 s, float2 dx, float2 dy, int2 texelOff) {
  return samp.SampleGrad(__builtin_sampler, s, dx, dy, texelOff);
}

float4 tex2DARRAYproj(sampler2DARRAY samp, float4 s) {
  return samp.Sample(__builtin_sampler, s.xyz / s.w);
}

float4 tex2DARRAYproj(sampler2DARRAY samp, float4 s, int2 texelOff) {
  return samp.Sample(__builtin_sampler, s.xyz / s.w, texelOff);
}

int4 tex2DARRAYproj(isampler2DARRAY samp, float4 s) {
  return samp.Sample(__builtin_sampler, s.xyz / s.w);
}

int4 tex2DARRAYproj(isampler2DARRAY samp, float4 s, int2 texelOff) {
  return samp.Sample(__builtin_sampler, s.xyz / s.w, texelOff);
}

uint4 tex2DARRAYproj(usampler2DARRAY samp, float4 s) {
  return samp.Sample(__builtin_sampler, s.xyz / s.w);
}

uint4 tex2DARRAYproj(usampler2DARRAY samp, float4 s, int2 texelOff) {
  return samp.Sample(__builtin_sampler, s.xyz / s.w, texelOff);
}

int3 tex2DARRAYsize(sampler2DARRAY samp, int lod) {
  uint width, height, elems, levels;
  samp.GetDimensions(lod, width, height, elems, levels);
  return int3(width, height, 0); // sic
}

int3 tex2DARRAYsize(isampler2DARRAY samp, int lod) {
  uint width, height, elems, levels;
  samp.GetDimensions(lod, width, height, elems, levels);
  return int3(width, height, 0); // sic
}

int3 tex2DARRAYsize(usampler2DARRAY samp, int lod) {
  uint width, height, elems, levels;
  samp.GetDimensions(lod, width, height, elems, levels);
  return int3(width, height, 0); // sic
}

float4 texCUBEARRAY(samplerCUBEARRAY samp, float4 s) {
  return samp.Sample(__builtin_sampler, s);
}

float4 texCUBEARRAY(samplerCUBEARRAY samp, float4 s, float3 dx, float3 dy) {
  return samp.SampleGrad(__builtin_sampler, s, dx, dy);
}

int4 texCUBEARRAY(isamplerCUBEARRAY samp, float4 s) {
  return samp.Sample(__builtin_sampler, s);
}

int4 texCUBEARRAY(isamplerCUBEARRAY samp, float4 s, float3 dx, float3 dy) {
  return samp.SampleGrad(__builtin_sampler, s, dx, dy);
}

uint4 texCUBEARRAY(usamplerCUBEARRAY samp, float4 s) {
  return samp.Sample(__builtin_sampler, s);
}

uint4 texCUBEARRAY(usamplerCUBEARRAY samp, float4 s, float3 dx, float3 dy) {
  return samp.SampleGrad(__builtin_sampler, s, dx, dy);
}

int3 texCUBEARRAYsize(samplerCUBEARRAY samp, int lod) {
  int width, height, elems, levels;
  samp.GetDimensions(lod, width, height, elems, levels);
  return int3(width, height, 0); // sic
}

int3 texCUBEARRAYsize(isamplerCUBEARRAY samp, int lod) {
  int width, height, elems, levels;
  samp.GetDimensions(lod, width, height, elems, levels);
  return int3(width, height, 0); // sic
}

int3 texCUBEARRAYsize(usamplerCUBEARRAY samp, int lod) {
  int width, height, elems, levels;
  samp.GetDimensions(lod, width, height, elems, levels);
  return int3(width, height, 0); // sic
}

float4 texBUF(samplerBUF samp, int s) {
  return samp.Load(s);
}

int4 texBUF(isamplerBUF samp, int s) {
  return samp.Load(s);
}

uint4 texBUF(usamplerBUF samp, int s) {
  return samp.Load(s);
}

uint texBUFsize(samplerBUF samp, int lod) {
  uint dim;
  samp.GetDimensions(dim);
  return dim;
}

uint texBUFsize(isamplerBUF samp, int lod) {
  uint dim;
  samp.GetDimensions(dim);
  return dim;
}

uint texBUFsize(usamplerBUF samp, int lod) {
  uint dim;
  samp.GetDimensions(dim);
  return dim;
}

// Redefine overloads for built-in texturing functions to match the Cg
// parameterization.  We can't overload tex2D, but we'll define them as
// f4tex2D et al and use macros to redefine tex2D et al to that.
float4 f4tex1D(sampler1D samp, float s) {
  return tex1D(samp, s);
}

float4 f4tex1D(sampler1D samp, float2 s) {
  return tex1D(samp, s.x);
}

float4 f4tex1D(sampler1DShadow samp, float2 s) {
  return samp.SampleCmp(__builtin_shadow_sampler, s.x, s.y);
}

float4 f4tex1D(sampler1DShadow samp, float2 s, int texelOff) {
  return samp.SampleCmp(__builtin_shadow_sampler, s.x, s.y, texelOff);
}

float4 f4tex1D(sampler1D samp, float s, float dx, float dy) {
  return tex1D(samp, s, dx, dy);
}

float4 f4tex1D(sampler1D samp, float2 s, float dx, float dy) {
  return tex1D(samp, s.x, dx, dy);
}

float4 f4tex1Dproj(sampler1D samp, float2 s) {
  return tex1Dproj(samp, s.xxxy);
}

float4 f4tex1Dproj(sampler1D samp, float3 s) {
  return tex1Dproj(samp, s.xxxz);
}

float4 f4tex1Dproj(sampler1DShadow samp, float3 s) {
  return samp.SampleCmp(__builtin_shadow_sampler, s.x / s.z, s.y / s.z);
}

float4 f4tex1Dproj(sampler1DShadow samp, float3 s, int texelOff) {
  return samp.SampleCmp(__builtin_shadow_sampler, s.x / s.z, s.y / s.z, texelOff);
}

float4 f4tex1Dproj(sampler1D samp, float2 s, float dx, float dy) {
  return tex1D(samp, s.x / s.y, dx, dy);
}

float4 f4tex1Dproj(sampler1D samp, float3 s, float dx, float dy) {
  return tex1D(samp, s.x / s.z);
}

float4 f4tex2D(sampler2D samp, float2 s) {
  return tex2D(samp, s.xy);
}

float4 f4tex2D(sampler2D samp, float3 s) {
  return tex2D(samp, s.xy);
}

float4 f4tex2D(sampler2DShadow samp, float3 s) {
  return samp.SampleCmp(__builtin_shadow_sampler, s.xy, s.z);
}

float4 f4tex2D(sampler2DShadow samp, float3 s, int2 texelOff) {
  return samp.SampleCmp(__builtin_shadow_sampler, s.xy, s.z, texelOff);
}

float4 f4tex2D(sampler2D samp, float2 s, float2 dx, float2 dy) {
  return tex2D(samp, s.xy, dx, dy);
}

float4 f4tex2D(sampler2D samp, float3 s, float2 dx, float2 dy) {
  return tex2D(samp, s.xy, dx, dy);
}

float4 f4tex2Dproj(sampler2D samp, float3 s) {
  return tex2Dproj(samp, s.xyzz);
}

float4 f4tex2Dproj(sampler2D samp, float4 s) {
  return tex2Dproj(samp, s);
}

float4 f4tex2Dproj(sampler2DShadow samp, float4 s) {
  return samp.SampleCmp(__builtin_shadow_sampler, s.xy / s.w, s.z / s.w);
}

float4 f4tex2Dproj(sampler2DShadow samp, float4 s, int2 texelOff) {
  return samp.SampleCmp(__builtin_shadow_sampler, s.xy / s.w, s.z / s.w, texelOff);
}

float4 f4tex2Dproj(sampler2D samp, float3 s, float2 dx, float2 dy) {
  return tex2D(samp, s.xyyz, dx, dy);
}

float4 f4tex2Dproj(sampler2D samp, float4 s, float2 dx, float2 dy) {
  return tex2D(samp, s.xy / s.w, dx, dy);
}

float4 f4texCUBE(samplerCUBE samp, float3 s) {
  return texCUBE(samp, s);
}

float4 f4texCUBE(samplerCUBE samp, float4 s) {
  return texCUBE(samp, s.xyz);
}

float4 f4texCUBE(samplerCubeShadow samp, float4 s) {
  return samp.SampleCmp(__builtin_shadow_sampler, s.xyz, s.w);
}

float4 f4texCUBE(samplerCUBE samp, float3 s, float3 dx, float3 dy) {
  return texCUBE(samp, s, dx, dy);
}

float4 f4texCUBE(samplerCUBE samp, float4 s, float3 dx, float3 dy) {
  return texCUBE(samp, s.xyz, dx, dy);
}

// Narrowing overloads (not defined for newer types)
#define f1tex1D(x, y) (f4tex1D((x), (y)).r)
#define f2tex1D(x, y) (f4tex1D((x), (y)).rg)
#define f3tex1D(x, y) (f4tex1D((x), (y)).rgb)
#define tex1D f4tex1D

#define f1tex1Dproj(x, y) (f4tex1Dproj((x), (y)).r)
#define f2tex1Dproj(x, y) (f4tex1Dproj((x), (y)).rg)
#define f3tex1Dproj(x, y) (f4tex1Dproj((x), (y)).rgb)
#define tex1Dproj f4tex1Dproj

#define f1tex2D(x, y) (f4tex2D((x), (y)).r)
#define f2tex2D(x, y) (f4tex2D((x), (y)).rg)
#define f3tex2D(x, y) (f4tex2D((x), (y)).rgb)
#define tex2D f4tex2D

#define f1tex2Dproj(x, y) (f4tex2Dproj((x), (y)).r)
#define f2tex2Dproj(x, y) (f4tex2Dproj((x), (y)).rg)
#define f3tex2Dproj(x, y) (f4tex2Dproj((x), (y)).rgb)
#define tex2Dproj f4tex2Dproj

#define f1texCUBE(x, y) (f4texCUBE((x), (y)).r)
#define f2texCUBE(x, y) (f2texCUBE((x), (y)).rg)
#define f3texCUBE(x, y) (f4texCUBE((x), (y)).rgb)
#define texCUBE f4texCUBE

// Legacy half-precision overloads
#define h1tex1D(x, y) (half(f1tex1D((x), (y))))
#define h2tex1D(x, y) (half2(f2tex1D((x), (y))))
#define h3tex1D(x, y) (half3(f3tex1D((x), (y))))
#define h4tex1D(x, y) (half4(f4tex1D((x), (y))))
#define h1tex1Dproj(x, y) (half(f1tex1Dproj((x), (y))))
#define h2tex1Dproj(x, y) (half2(f2tex1Dproj((x), (y))))
#define h3tex1Dproj(x, y) (half3(f3tex1Dproj((x), (y))))
#define h4tex1Dproj(x, y) (half4(f4tex1Dproj((x), (y))))
#define h1tex2D(x, y) (half(f1tex2D((x), (y))))
#define h2tex2D(x, y) (half2(f2tex2D((x), (y))))
#define h3tex2D(x, y) (half3(f3tex2D((x), (y))))
#define h4tex2D(x, y) (half4(f4tex2D((x), (y))))
#define h1tex2Dproj(x, y) (half(f1tex2Dproj((x), (y))))
#define h2tex2Dproj(x, y) (half2(f2tex2Dproj((x), (y))))
#define h3tex2Dproj(x, y) (half3(f3tex2Dproj((x), (y))))
#define h4tex2Dproj(x, y) (half4(f4tex2Dproj((x), (y))))
#define h1tex3D(x, y) (half(f1tex3D((x), (y))))
#define h2tex3D(x, y) (half2(f2tex3D((x), (y))))
#define h3tex3D(x, y) (half3(f3tex3D((x), (y))))
#define h4tex3D(x, y) (half4(f4tex3D((x), (y))))
#define h1texCUBE(x, y) (half(f1texCUBE((x), (y))))
#define h2texCUBE(x, y) (half2(f2texCUBE((x), (y))))
#define h3texCUBE(x, y) (half3(f3texCUBE((x), (y))))
#define h4texCUBE(x, y) (half4(f4texCUBE((x), (y))))

// Legacy fixed-precision overloads
#define x1tex1D(x, y) (fixed(f1tex1D((x), (y))))
#define x2tex1D(x, y) (fixed2(f2tex1D((x), (y))))
#define x3tex1D(x, y) (fixed3(f3tex1D((x), (y))))
#define x4tex1D(x, y) (fixed4(f4tex1D((x), (y))))
#define x1tex1Dproj(x, y) (fixed(f1tex1Dproj((x), (y))))
#define x2tex1Dproj(x, y) (fixed2(f2tex1Dproj((x), (y))))
#define x3tex1Dproj(x, y) (fixed3(f3tex1Dproj((x), (y))))
#define x4tex1Dproj(x, y) (fixed4(f4tex1Dproj((x), (y))))
#define x1tex2D(x, y) (fixed(f1tex2D((x), (y))))
#define x2tex2D(x, y) (fixed2(f2tex2D((x), (y))))
#define x3tex2D(x, y) (fixed3(f3tex2D((x), (y))))
#define x4tex2D(x, y) (fixed4(f4tex2D((x), (y))))
#define x1tex2Dproj(x, y) (fixed(f1tex2Dproj((x), (y))))
#define x2tex2Dproj(x, y) (fixed2(f2tex2Dproj((x), (y))))
#define x3tex2Dproj(x, y) (fixed3(f3tex2Dproj((x), (y))))
#define x4tex2Dproj(x, y) (fixed4(f4tex2Dproj((x), (y))))
#define x1tex3D(x, y) (fixed(f1tex3D((x), (y))))
#define x2tex3D(x, y) (fixed2(f2tex3D((x), (y))))
#define x3tex3D(x, y) (fixed3(f3tex3D((x), (y))))
#define x4tex3D(x, y) (fixed4(f4tex3D((x), (y))))
#define x1texCUBE(x, y) (fixed(f1texCUBE((x), (y))))
#define x2texCUBE(x, y) (fixed2(f2texCUBE((x), (y))))
#define x3texCUBE(x, y) (fixed3(f3texCUBE((x), (y))))
#define x4texCUBE(x, y) (fixed4(f4texCUBE((x), (y))))

// Bit manipulation functions
uint bitfieldExtract(uint val, int off, int size) {
  uint mask = uint((1 << size) - 1);
  return uint(val >> off) & mask;
}

#define bitCount(x) (countbits(x))
#define bitfieldReverse(x) (reversebits(x))
#define findLSB(x) (firstbitlow(x))
#define findMSB(x) (firstbithigh(x))
#define floatToIntBits(x) (asint(x))
#define floatToRawIntBits(x) (asint(x))
#define intBitsToFloat(x) (asfloat(x))

// Matrix inverse functions
float2x2 inverse(float2x2 m) {
  return float2x2(m._22, -m._12, -m._21, m._11) * (1.0f / determinant(m));
}

float3x3 inverse(float3x3 m) {
  float3 t = float3(m._22 * m._33 - m._23 * m._32,
                    m._23 * m._31 - m._21 * m._33,
                    m._21 * m._32 - m._22 * m._31);
  return float3x3(
    t[0], m._13 * m._32 - m._12 * m._33, m._12 * m._23 - m._13 * m._22,
    t[1], m._11 * m._33 - m._13 * m._31, m._13 * m._21 - m._11 * m._23,
    t[2], m._12 * m._31 - m._11 * m._32, m._11 * m._22 - m._12 * m._21
  ) * (1.0f / dot(m[0], t));
}

float4x4 inverse(float4x4 m) {
  float4 t = float4(
    m._32 * m._43 * m._24 - m._42 * m._33 * m._24 + m._42 * m._23 * m._34 - m._22 * m._43 * m._34 - m._32 * m._23 * m._44 + m._22 * m._33 * m._44,
    m._41 * m._33 * m._24 - m._31 * m._43 * m._24 - m._41 * m._23 * m._34 + m._21 * m._43 * m._34 + m._31 * m._23 * m._44 - m._21 * m._33 * m._44,
    m._31 * m._42 * m._24 - m._41 * m._32 * m._24 + m._41 * m._22 * m._34 - m._21 * m._42 * m._34 - m._31 * m._22 * m._44 + m._21 * m._32 * m._44,
    m._41 * m._32 * m._23 - m._31 * m._42 * m._23 - m._41 * m._22 * m._33 + m._21 * m._42 * m._33 + m._31 * m._22 * m._43 - m._21 * m._32 * m._43);
  return float4x4(
      t[0],
      (m._42 * m._33 * m._14 - m._32 * m._43 * m._14 - m._42 * m._13 * m._34 + m._12 * m._43 * m._34 + m._32 * m._13 * m._44 - m._12 * m._33 * m._44),
      (m._22 * m._43 * m._14 - m._42 * m._23 * m._14 + m._42 * m._13 * m._24 - m._12 * m._43 * m._24 - m._22 * m._13 * m._44 + m._12 * m._23 * m._44),
      (m._32 * m._23 * m._14 - m._22 * m._33 * m._14 - m._32 * m._13 * m._24 + m._12 * m._33 * m._24 + m._22 * m._13 * m._34 - m._12 * m._23 * m._34),

      t[1],
      (m._31 * m._43 * m._14 - m._41 * m._33 * m._14 + m._41 * m._13 * m._34 - m._11 * m._43 * m._34 - m._31 * m._13 * m._44 + m._11 * m._33 * m._44),
      (m._41 * m._23 * m._14 - m._21 * m._43 * m._14 - m._41 * m._13 * m._24 + m._11 * m._43 * m._24 + m._21 * m._13 * m._44 - m._11 * m._23 * m._44),
      (m._21 * m._33 * m._14 - m._31 * m._23 * m._14 + m._31 * m._13 * m._24 - m._11 * m._33 * m._24 - m._21 * m._13 * m._34 + m._11 * m._23 * m._34),

      t[2],
      (m._41 * m._32 * m._14 - m._31 * m._42 * m._14 - m._41 * m._12 * m._34 + m._11 * m._42 * m._34 + m._31 * m._12 * m._44 - m._11 * m._32 * m._44),
      (m._21 * m._42 * m._14 - m._41 * m._22 * m._14 + m._41 * m._12 * m._24 - m._11 * m._42 * m._24 - m._21 * m._12 * m._44 + m._11 * m._22 * m._44),
      (m._31 * m._22 * m._14 - m._21 * m._32 * m._14 - m._31 * m._12 * m._24 + m._11 * m._32 * m._24 + m._21 * m._12 * m._34 - m._11 * m._22 * m._34),

      t[3],
      (m._31 * m._42 * m._13 - m._41 * m._32 * m._13 + m._41 * m._12 * m._33 - m._11 * m._42 * m._33 - m._31 * m._12 * m._43 + m._11 * m._32 * m._43),
      (m._41 * m._22 * m._13 - m._21 * m._42 * m._13 - m._41 * m._12 * m._23 + m._11 * m._42 * m._23 + m._21 * m._12 * m._43 - m._11 * m._22 * m._43),
      (m._21 * m._32 * m._13 - m._31 * m._22 * m._13 + m._31 * m._12 * m._23 - m._11 * m._32 * m._23 - m._21 * m._12 * m._33 + m._11 * m._22 * m._33)
    ) * (1.0f / dot(m[0], t));
}
