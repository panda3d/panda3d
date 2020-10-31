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

// We implement shadow samplers by mapping them to DX10 syntax, since there is
// no syntax for them in DX9.
typedef Texture1D sampler1DShadow;
typedef Texture2D sampler2DShadow;
typedef TextureCube samplerCubeShadow;

SamplerComparisonState __builtin_shadow_sampler;

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

float4 f4tex2D(sampler2DShadow samp, float3 s, int texelOff) {
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

float4 f4tex2Dproj(sampler2DShadow samp, float4 s, int texelOff) {
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

#define f1tex1D(x, y) (f4tex1D((x), (y)).r)
#define f3tex1D(x, y) (f4tex1D((x), (y)).rgb)
#define tex1D f4tex1D

#define f1tex1Dproj(x, y) (f4tex1Dproj((x), (y)).r)
#define f3tex1Dproj(x, y) (f4tex1Dproj((x), (y)).rgb)
#define tex1Dproj f4tex1Dproj

#define f1tex2D(x, y) (f4tex2D((x), (y)).r)
#define f3tex2D(x, y) (f4tex2D((x), (y)).rgb)
#define tex2D f4tex2D

#define f1tex2Dproj(x, y) (f4tex2Dproj((x), (y)).r)
#define f3tex2Dproj(x, y) (f4tex2Dproj((x), (y)).rgb)
#define tex2Dproj f4tex2Dproj

#define f1texCUBE(x, y) (f4texCUBE((x), (y)).r)
#define f3texCUBE(x, y) (f4texCUBE((x), (y)).rgb)
#define texCUBE f4texCUBE

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
